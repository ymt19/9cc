#include "9cc.h"

// ローカル変数
LVar *locals;

//変数を名前で検索する
//見つからなかったら、NULLを返す
LVar *find_lvar(Token *tok) {
    for (LVar *lvar = locals; lvar; lvar = lvar->next) {
        if (strlen(lvar->name) == tok->len && !strncmp(lvar->name, tok->str, tok->len))
            return lvar;
    }
    return NULL;
}

Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// 数値のnodeの生成
Node *new_node_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

// ローカル変数の生成
Node *new_lvar_node(LVar *lvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
	node->lvar = lvar;
    return node;
}

LVar *new_lvar(char *name) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = name;
    locals = lvar;
    return lvar;
}

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *umary();
Node *primary();

// program = stmt*
Function *program() {
    locals = NULL;

    Node head;
	head.next = NULL;
	Node *cur = &head;
	
    while (!at_eof()) {
		cur->next = stmt();
		cur = cur->next;
    }
	
	Function *prog = calloc(1, sizeof(Function));
	prog->node = head.next;
	prog->locals = locals;
	return prog;
}

/* stmt = expr ";" 
		| "if" "(" expr ")" stmt ("else" stmt)?
		| "while" "(" expr ")" stmt
		| "for" "(" expr? ";" expr? ";" expr? ")" stmt
		| "return" expr ";"
*/
Node *stmt() {
	Node *node;

	if(consume("if")) {
		node = new_node(ND_IF);
		expect("(");
		node->cond_expr = expr();
		expect(")");
		node->then = stmt();
		if(consume("else")) {
			node->els = stmt();
		}
		return node;
	}

	if(consume("while")) {
		node = new_node(ND_WHILE);
		expect("(");
		node->cond_expr = expr();
		expect(")");
		node->then = stmt();
		return node;
	}

	if(consume("for")) {
		node = new_node(ND_FOR);
		expect("(");
		if(!consume(";")) {
			node->init = expr();
			expect(";");
		}
		if(!consume(";")) {
			node->cond_expr = expr();
			expect(";");
		}
		if(!consume(")")) {
			node->re_init = expr();
			expect(")");
		}
		node->then = stmt();
		return node;
	}

	if(consume("return")) {
		node = new_node(ND_RETURN);
		node->lhs = expr();
		expect(";");
		return node;
	}

	node = expr();
	expect(";");
	return node;
}

//expr = assign
Node *expr() {
	return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality();
    if(consume("=")) {
        node = new_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

//equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
	Node *node = relational();

	for(;;) {
		if(consume("==")) {
			node = new_binary(ND_EQ, node, relational());
		}
		else if(consume("!=")) {
			node = new_binary(ND_NE, node, relational());
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
			node = new_binary(ND_LT, node, add());
		}
		else if(consume("<=")){
			node = new_binary(ND_LE, node, add());
		}
		else if(consume(">")) {
			node = new_binary(ND_LT, add(), node);
		}
		else if(consume(">=")) {
			node = new_binary(ND_LE, add(), node);
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
			node = new_binary(ND_ADD, node, mul());
		}
		else if(consume("-")) {
			node = new_binary(ND_SUB, node, mul());
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
			node = new_binary(ND_MUL, node, umary());
		}
		else if(consume("/")) {
			node = new_binary(ND_DIV, node, umary());
		}
		else
			return node;
	}
}

//unary = ("+" | "-")? primary44redxkm
Node *umary() {
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_binary(ND_SUB, new_node_num(0), primary());
	return primary();
}

//primary = num | ident | "(" expr ")"
Node *primary() {
	//次のトークンが'('なら、'(' expr ')' のはず
	if(consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

    //ND_LVARのとき
    Token *tok = consume_ident();
    if(tok) {
        LVar *lvar = find_lvar(tok);
        if(!lvar) {
            char *name;
			name = calloc(1, sizeof(char) * sizeof(tok->len));
            strncpy(name, tok->str, tok->len);
            lvar = new_lvar(name);
        }
        return new_lvar_node(lvar);
    }

	//ND_NUMのとき
	return new_node_num(expect_number());
}