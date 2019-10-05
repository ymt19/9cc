#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//
// parse.c
//

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

//トークン型
typedef struct Token Token;
struct Token
{
	TokenKind kind; // トークンの型
	Token *next;    // 次の入力トークン
	int val;        //kindがTK_NUMの場合、その数値
	char *str;      //トークン文字列
	int len;        //トークンの長さ
};

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

//抽象構文木のノードの種類
typedef enum {
	ND_ADD,     // +
	ND_SUB,     // -
	ND_MUL,     // *
	ND_DIV,     // /
	ND_EQ,      // ==
	ND_NE,      // !=
	ND_LT,      // <
	ND_LE,      // <=
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
	ND_NUM,     // 整数
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind;  //ノードの型
	Node *lhs;      //左辺 left hand side
	Node *rhs;      //右辺 right hand side
	int val;        //kindがND_NUMの場合のみ
    int offset;     //kindがND_LVARの場合のみ
};

Token *tokenize();
Node *expr();

// セミコロンで区切った複数の式のパースの結果
Node *code[100];


//
// codegen.c
//

void gen(Node *node);
