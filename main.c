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
    program();
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("  pop rax\n");
    }

    //epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    // return from function
    printf("  ret\n");
    return 0;
}
