#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED, // symbol
    TK_IDENT, // identifier
    TK_NUM,      // num
    TK_EOF,      // EOF
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // type of token
    Token *next;
    int val; // if tolen is num, val
    char *str;
    int len;
};
// parser
typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASSIGN, // =
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_LVAR, // local_val
    ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node
{
    NodeKind kind; // Node kind
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // Used if kind == ND_NUM
    int offset;     // Used if kind == ND_LVAR
};

extern char *user_input;
extern Token *token;
extern Node *code[100];

void gen(Node *node);
void error(char *fmt, ...);
Token *tokennize();
Token *consume_ident();

Node *expr();
void program();
