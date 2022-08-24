#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	TK_RESERVED,	//記号
	TK_NUM,  	 	//整数トークン
	TK_EOF,  		//入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
	TokenKind kind; //トークンの型
	Token *next; 	//次の入力のトークン
	int val; 		//kindがTK_NUMの場合, その数値
	char *str; 		//トークン文字列
};

char *user_input;
Token *token;

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//次のトークンが期待している記号の時には、トークンを一つ読み進めてTrue.
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

//次のトークンが期待している記号の場合、トークンを一つ読み進める。それ以外はエラーを返す。
int expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str, "expected '%c'", op);
	token = token->next;
}

//次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す。
int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してCurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// 入力文字列pをトークンナイズしてそれを返す
Token *tokennize() {
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}
		if(strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}


//parser
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
	NodeKind kind; // Node kind
	Node *lhs; 		// Left-hand side
	Node *rhs; 		// Right-hand side
	int val; 		// Used if kind == ND_NUM
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

Node *mul();
Node *primary();

// expr = mul('+' mul | "-" mul)*
Node *expr() {
	Node *node = mul();

	for(;;) {
		if (consume('+')) {
			node = new_node(ND_ADD, node, mul());
		}
		else if (consume('-')) {
			node = new_node(ND_SUB, node, mul());
		}
		else {
			return node;
		}
	}
}

// mul = primary("*" primary | "/" primary)*
Node *mul() {
	Node *node = primary();
	for(;;) {
		if (consume('*')) {
			node = new_node(ND_MUL, node, primary());
		}
		else if (consume('/')) {
			node = new_node(ND_DIV, node, primary());
		}
		else {
			return node;
		}
	}
}

// primary = "(" expr ")" | num

Node *primary() {
	if(consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}

	return new_node_num(expect_number());
}

// code gen(stack machine)
void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}
	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf("  add rax, rdi\n");
			break;
		case ND_SUB:
			printf("  sub rax, rdi\n");
			break;
		case ND_MUL:
			printf(" imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("  cqo\n");
			printf("  idiv rdi\n");
			break;
	}

	printf("  push rax\n");
}


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr , "引数の個数が正しくない\n");
		return 1;
	}

	user_input = argv[1];
	token = tokennize();
	Node *node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	gen(node);

	// return  value in stack top
	printf("  pop rax\n");
	// return from function
	printf(" 	ret\n");
	return 0;
}

