#include "9cc.h"

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "引数が正しくない\n");
		return 1;
	}

	user_input = argv[1];

	//トークナイズする
	token = tokenize();

	//パースする
	Node *node = expr();

	//アセンブリの前半を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//抽象構文木からアセンブリ出力
	gen(node);

	//スタックトップには計算結果があるはずなので
	//RAXにロードする
	printf("	pop rax\n");
	printf("    ret\n");
	return 0;
}