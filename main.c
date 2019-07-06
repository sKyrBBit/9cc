#include "9cc.h"

/// memory management : only allocate, not free
// TODO not repeat yourself
// TODO divide high and low layers

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
void error_at(const char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
void prologue(const int lvals) {
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", lvals);
}
void epilogue() {
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect number of arguments");
        return 1;
    }

    user_input = argv[1];
    tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    prologue(locals_len() * 8);
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("  pop rax\n"); // TODO remove?
    }
    epilogue();

    return 0;
}