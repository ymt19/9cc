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
	Function *prog = program();

	//変数に必要なオフセットを決定する
	int offset = 0;
	for (LVar *lvar = prog->locals; lvar; lvar = lvar->next) {
		offset += 8;
		lvar->offset = offset;
	}
	prog->stack_size = offset;

	//アセンブリ出力
	codegen(prog);


	return 0;
}