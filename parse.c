#include "9cc.h"

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
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		return false;

	token = token->next;
	return true;
}

//つぎのトークンが期待している記号のときは、トークンを１つ読み進める
//それ以外の場合にはエラーを報告
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

//２つの文字列の比較
//pの先頭の文字列とq全体の文字列の比較
bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
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

		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">=")) {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
		
		if (strchr("+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		//数値の場合
		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);

			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(token->str, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// 数値の場合
Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}


Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *umary();
Node *primary();

//expr       = equality
Node *expr() {
	return equality();
}

//equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
	Node *node = relational();

	for(;;) {
		if(consume("==")) {
			node = new_node(ND_EQ, node, relational());
		}
		else if(consume("!=")) {
			node = new_node(ND_NE, node, relational());
		}
		else
			return node;
	}
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
	Node *node = add();

	for(;;) {
		if(consume("<")) {
			node = new_node(ND_LT, node, add());
		}
		else if(consume("<=")){
			node = new_node(ND_LE, node, add());
		}
		else if(consume(">")) {
			node = new_node(ND_LT, add(), node);
		}
		else if(consume(">=")) {
			node = new_node(ND_LE, add(), node);
		}
		else
			return  node;
	}	
}

//add = mul ("+" mul | "-" mul)*
Node *add() {
	Node *node = mul();

	for(;;) {
		if(consume("+")) {
			node = new_node(ND_ADD, node, mul());
		}
		else if(consume("-")) {
			node = new_node(ND_SUB, node, mul());
		}
		else
			return node;
	}
}

//mul = unary ("*" unary | "/" unary)*
Node *mul() {
	Node *node = umary();

	for(;;) {
		if(consume("*")) {
			node = new_node(ND_MUL, node, umary());
		}
		else if(consume("/")) {
			node = new_node(ND_DIV, node, umary());
		}
		else
			return node;
	}
}

//unary = ("+" | "-")? primary
Node *umary() {
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

//primary = num | "(" expr ")"
Node *primary() {
	//次のトークンが'('なら、'(' expr ')' のはず
	if(consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	//そうでなければ数値
	return new_node_num(expect_number());
}