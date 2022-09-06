#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	TK_RESERVED,	//symbol
	TK_NUM,  	 	//num
	TK_EOF,  		//EOF
} TokenKind;

typedef struct Token Token;

struct Token {
	TokenKind kind; //type of token
	Token *next;
	int val; 		//if tolen is num, val
	char *str;
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
	fprintf(stderr, "%*s", pos, " "); //output blank number of pos
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// if token is symbol, read forward a token and return true
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

//if token is mark, read forward one token
int expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str, "expected '%c'", op);
	token = token->next;
}

//if next token is num, return this num and read forward one token
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

// gen new token and connect Cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

//  tokennize input p and return p
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
Node *unary();

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

// mul = unary("*" unary | "/" unary)*
Node *mul() {
	Node *node = unary();
	for(;;) {
		if (consume('*')) {
			node = new_node(ND_MUL, node, unary());
		}
		else if (consume('/')) {
			node = new_node(ND_DIV, node, unary());
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

// unary = ("+" | "-")? primary
Node *unary() {
	if(consume('+'))
		return primary();
	if(consume('-'))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
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

