#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token
{
	TokenKind kind; // トークンの型
	Token *next;    // 次の入力トークン
	int val;        //kindがTK_NUMの場合、その数値
	char *str;      //トークン文字列;
};

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//エラー箇所の報告する関数
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); //pos回空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// つぎのトークンが期待している記号の時には、トークンを１つ読み進めて
// 真を返す。それ以外の場合には偽を返す
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

//つぎのトークンが期待している記号のときは、トークンを１つ読み進める
//それ以外の場合にはエラーを報告
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

//つぎのトークンが数値の場合、トークンを１つ読み進めてその数値を返す
//それ以外の場合はエラーを返す
int expect_number(){
	if (token->kind != TK_NUM)
		error_at(token->str, "数値ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
	Token head;
	head.next = NULL;
	Token *cur = &head;

	char *p = user_input;

	while (*p) {
		//空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		if(*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(token->str, "トークナイズ出来ません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

//抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind; //ノードの型
	Node *lhs;     //左辺 left hand side
	Node *rhs;     //右辺 right hand side
	int val;       //整数の場合のみ
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr() {
	Node *node = mul();

	for(;;) {
		if(consume('+')) {
			node = new_node(ND_ADD, node, mul());
		}
		else if(consume('-')) {
			node = new_node(ND_SUB, node, mul());
		}
		else
			return node;
	}
}

Node *mul() {
	Node *node = primary();

	for(;;) {
		if(consume('*')) {
			node = new_node(ND_MUL, node, primary());
		}
		else if(consume('/')) {
			node = new_node(ND_DIV, node, primary());
		}
		else
			return node;
	}
}

Node *primary() {
	//次のトークンが'('なら、'(' expr ')' のはず
	if(consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}

	//そうでなければ数値
	return new_node_num(expect_number());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("    push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->kind)
	{
	case ND_ADD:
		printf("	add rax, rdi\n");
		break;
	case ND_SUB:
		printf("	sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("	imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv rdi\n");
		break;
	}

	printf("	push rax\n");
}


int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "引数が正しくない\n");
		return 1;
	}

	user_input = argv[1];

	//トークナイズする
	token = tokenize();

	//アセンブリの前半を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//式の最初が数値かどうかの確認
	//mov命令出力
	printf("    mov rax, %d\n", expect_number());

	// +数値 -数値というトークンの並びを消費して
	//アセンブリ出力
	while (!at_eof())
	{
		if(consume('+')) {
			printf("    add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("    sub rax, %d\n", expect_number());
	}

	printf("    ret\n");
	return 0;
	
}