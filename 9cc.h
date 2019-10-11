#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//
// tokenize.c
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
struct Token {
	TokenKind kind; // トークンの型
	Token *next;    // 次の入力トークン
	int val;        //kindがTK_NUMの場合、その数値
	char *str;      //トークン文字列
	int len;        //トークンの長さ
};

// 現在着目しているトークン
Token *token;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();

// 入力プログラム
char *user_input;

Token *tokenize();



//
// parse.c
//

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

//ローカル変数の型
typedef struct LVar LVar;
struct LVar {
    LVar *next; // 次の変数
    char *name; // 変数名
    int offset; // RBP(ベースレジスタ)からのオフセット
};

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind;  //ノードの型
	Node *next; 	//次のノード
	Node *lhs;      //左辺 left hand side
	Node *rhs;      //右辺 right hand side
	int val;        //kindがND_NUMの場合のみ
	LVar *lvar;		//kindがND_LVARの場合のみ
};

typedef struct Function Function;
struct Function {
	Node *node;
	LVar *locals;
	int stack_size;
};

Function *program();



//
// codegen.c
//

void codegen();
