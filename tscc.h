#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED, // symbol
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
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
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
};

extern char *user_input;
extern Token *token;

void gen(Node *node);
Token *tokennize();
Node *expr();
