#include "9cc.h"

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "引数が正しくない\n");
		return 1;
	}

	user_input = argv[1];

	//トークナイズする
    //結果はcodeに保存される
	token = tokenize();

	//パースする
	Node *node = expr();

	//アセンブリの前半を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

    //プロローグ
    //変数26個分の領域を確保する
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");   // rsp <--26*8bite--> rbp

	//先頭の式からコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        //式の結果がスタックに残っているはずなので、
        //スタックが溢れないようポップする
        printf("    pop rax\n");
    }

	//エピローグ
    //最後の指揮の結果がRAXに残っているので、それが返り値
    printf("    mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("    ret\n");
	return 0;
}