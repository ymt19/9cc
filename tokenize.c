#include "9cc.h"

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

// つぎのトークンがNK_IDENTのとき、トークンを１つ読み進めて
// そのトークンを返す。それ以外はNULLを返す
Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    
    // 返り値となるNK_IDENTのトークン
    Token *token_ident = token;
    token = token->next;
    return token_ident;
}

//つぎのトークンが期待している記号のときは、トークンを１つ読み進める
//それ以外の場合にはエラーを報告
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str, "'%c'ではありません", *op);
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

//アルファベット、'_'を引数に渡したらtrue
bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

//is_alphaに加え数値も許可する
bool is_alpha_num(char c) {
	return is_alpha(c) || (c <= '1' && '9' <= c);
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

        //2文字の記号
		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">=")) {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
		
        //1文字の記号
		if (strchr("+-*/()<>=;", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
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

        // TK_IDENT型トークンの場合
        if (is_alpha(*p)) {
            char *q = p;
            p++;
            while (is_alpha_num(*p)) {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

		error_at(token->str, "invalid token");
	}


	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
