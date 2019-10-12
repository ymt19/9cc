#include "9cc.h"

// 変数分のアドレスをスタックにpushする
void gen_lvar(Node *node) {
    if (node->kind == ND_LVAR) {
		printf("	lea rax, [rbp-%d]\n", node->lvar->offset);
		printf("	push rax\n");
		return;
	}
	error("not a value");
}

//popしたアドレスから参照できる値をpushする
//先頭の値はアドレス扱いの値である
void load() {
	printf("	pop rax\n");
	printf("	mov rax, [rax]\n");
	printf("	push rax\n");
}

//先頭の値をその次の値（アドレス）に代入する
void store() {
	printf("	pop rdi\n");
	printf("	pop rax\n");
	printf("	mov [rax], rdi\n");
	printf("	push rdi\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lvar(node);
        load();
        return;
    case ND_ASSIGN:
        gen_lvar(node->lhs);
        gen(node->rhs);
		store();
		return;
	case ND_RETURN:
		gen(node->lhs);
		printf("	pop rax\n");
		printf("	mov rsp, rbp\n");
		printf("	pop rbp\n");
		printf("	ret\n");
		return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

	switch (node->kind) {
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
	case ND_EQ:
		printf("	cmp rax, rdi\n");
		printf("	sete al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_NE:
		printf("	cmp rax, rdi\n");
		printf("	setne al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_LT:
		printf("	cmp rax, rdi\n");
		printf("	setl al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_LE:
		printf("	cmp rax, rdi\n");
		printf("	setle al\n");
		printf("	movzb rax, al\n");
		break;
	}

	printf("	push rax\n");
}

void codegen(Function *prog) {
	//アセンブリの前半を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//プロローグ
    //変数26個分の領域を確保する
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", prog->stack_size);   // rsp <--スタックサイズ--> rbp

	//先頭の式からコード生成
    for (Node *node = prog->node; node; node = node->next) {
		gen(node);
    }

	//エピローグ
    //最後の式の結果がRAXに残っているので、それが返り値
    printf("    mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("    ret\n");
}