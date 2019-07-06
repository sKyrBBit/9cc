#include "9cc.h"

// lexical analyzer
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}
Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}
bool consume_ret() {
    if (token->kind != TK_RET) {
        return false;
    }
    token = token->next;
    return true;
}
void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "not a character, '%c'", op);
    }
    token = token->next;
}
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "not a number");
    int val = token->val;
    token = token->next;
    return val;
}
bool at_eof() {
    return token->kind == TK_EOF;
}
void tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (strncmp(p, ">=", 2) == 0 ||
            strncmp(p, "<=", 2) == 0 ||
            strncmp(p, "==", 2) == 0 ||
            strncmp(p, "!=", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += cur->len;
            continue;
        }
        if (strncmp(p, "+", 1) == 0 ||
            strncmp(p, "-", 1) == 0 ||
            strncmp(p, "*", 1) == 0 ||
            strncmp(p, "/", 1) == 0 ||
            strncmp(p, ">", 1) == 0 ||
            strncmp(p, "<", 1) == 0 ||
            strncmp(p, "=", 1) == 0 ||
            strncmp(p, ";", 1) == 0) {
            cur = new_token(TK_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }
        if (strncmp(p, "return", 6) == 0 && !(isalnum(p[6]) || p[6] == '_')) {
            cur = new_token(TK_RET, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }
        if (isalpha(*p)) {
            int len = 1;
            char *q = p + 1;
            while (isalnum(*q) || q[0] == '_') {
                len++;
                q++;
            }
            cur = new_token(TK_IDENT, cur, p);
            cur->len = len;
            p += len;
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            int len = strlen(p);
            cur->val = strtol(p, &p, 10);
            cur->len = len - strlen(p);
            continue;
        }
        error("unable to tokenize"); // TODO review
    }
    new_token(TK_EOF, cur, p);
    token = head.next;
}
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}
int locals_len() {
    int len = 0;
    for (LVar *var = locals; var; len++) {
        var = var->next;
    }
    return len;
}

// parser
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
void program() {
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}
Node *stmt() {
    Node *node;
    if (consume_ret()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        node->lhs = expr();
    } else {
        node = expr();
    }
    expect(";");
    return node;
}
Node *expr() {
    return assign();
}
Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASS, node, assign());
    return node;
}
Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("=="))
            node = new_node(ND_E, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">="))
            node = new_node(ND_GE, node, add());
        else if (consume("<"))
            node = new_node(ND_L, node, add());
        else if (consume(">"))
            node = new_node(ND_G, node, add());
        else
            return node;
    }
}
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}
Node *unary() {
    if (consume("+"))
        return term();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), term());
    return term();
}
Node *term() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            if (!locals) lvar->offset = 8; // TODO review
            else lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    return new_node_num(expect_number());
}