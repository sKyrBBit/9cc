#ifndef INC_9CC_9CC_H
#define INC_9CC_9CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO remove unnecessary declaration

// parse.c
typedef enum {
    TK_RESERVED,
    TK_RET,
    TK_IDENT,
    TK_NUM,
    TK_EOF
} TokenKind;
typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *token;
typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};
LVar *locals;
bool consume(char *op);
Token *consume_ident();
bool consume_ret();
void expect(char *op);
int expect_number();
bool at_eof();
void tokenize();
LVar *find_lvar(Token *tok);
int locals_len();

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASS, // =
    ND_RET, // return
    ND_LVAR,
    ND_NUM,

    // optional
    ND_E,
    ND_NE,
    ND_L,
    ND_G,
    ND_LE,
    ND_GE
} NodeKind;
typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs; // left-hand side
    Node *rhs; // right-hand side
    int val;
    int offset;
};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *code[100];

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

// codegen.c
void gen_lval(Node *node);
void gen(Node *node);

// main.c
char *user_input;
void error(char *fmt, ...);
void error_at(const char *loc, char *fmt, ...);
void prologue(const int lvals);
void epilogue();

#endif //INC_9CC_9CC_H
