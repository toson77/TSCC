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
	int len;
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
bool consume(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

//if token is mark, read forward one token
int expect(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswitch(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
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
		if(startswitch(p, "==") || startswitch(p, "!=") || startswitch(p, "<=") || startswitch(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}
		if(strchr("+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}


//parser
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ,  // ==
	ND_NE,  // !=
	ND_LT,  // <
	ND_LE,  // <=
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

Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr() {
	return equality();
}

// equality = relational( "==" relational | "!=" relational )*
Node *equality() {
	Node *node = relational();

	for(;;) {
		if(consume("==")) {
			node = new_node(ND_EQ, node, relational());
		}
		else if(consume("!=")) {
			node = new_node(ND_NE, node, relational());
		}
		else {
			return node;
		}
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
	Node *node = add();
	for(;;) {
		if(consume("<")) {
			node = new_node(ND_LT, node, add());
		}
		else if(consume(">")) {
			node = new_node(ND_LT, add(), node);
		}
		else if(consume("<=")) {
			node = new_node(ND_LE, node, add());
		}
		else if(consume(">=")) {
			node = new_node(ND_LE, add(), node);
		}
		else {
			return node;
		}
	}
}

// add = mul("+" mul | "-" mul)*
Node *add() {
	Node *node = mul();
	for(;;) {
		if(consume("+")) {
			node = new_node(ND_ADD, node, mul());
		}
		else if(consume("-")) {
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
		if (consume("*")) {
			node = new_node(ND_MUL, node, unary());
		}
		else if (consume("/")) {
			node = new_node(ND_DIV, node, unary());
		}
		else {
			return node;
		}
	}
}

// unary = ("+" | "-")? primary
Node *unary() {
	if(consume("+"))
		return primary();
	if(consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

// primary = "(" expr ")" | num

Node *primary() {
	if(consume("(")) {
		Node *node = expr();
		expect(")");
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
		case ND_EQ:
			printf("  cmp rax, rdi\n");
			printf("  sete al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_NE:
			printf("  cmp rax, rdi\n");
			printf("  setne al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LT:
			printf("  cmp rax, rdi\n");
			printf("  setl al\n");
			printf("  movzb rax, al\n");
			break;
		case ND_LE:
			printf("  cmp rax, rdi\n");
			printf("  setle al\n");
			printf("  movzb rax, al\n");
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

