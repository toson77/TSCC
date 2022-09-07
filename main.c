#include "tscc.h"

Token *token;
char *user_input;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくない\n");
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
