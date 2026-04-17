#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 1024
#define MAX_TEXT 4096

typedef enum {
    TOK_NUMBER,
    TOK_IDENT,
    TOK_OP,
    TOK_LPAREN,
    TOK_RPAREN
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
    int number;
    char op;
} Token;

typedef enum {
    NODE_NUMBER,
    NODE_IDENT,
    NODE_OP
} NodeType;

typedef struct Node {
    NodeType type;
    int number;
    char ident[64];
    char op;
    struct Node* left;
    struct Node* right;
} Node;

void die(const char* msg) {
    fprintf(stderr, "Ошибка: %s\n", msg);
    exit(1);
}

Node* new_number(int x) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) die("malloc");
    n->type = NODE_NUMBER;
    n->number = x;
    n->ident[0] = '\0';
    n->op = 0;
    n->left = NULL;
    n->right = NULL;
    return n;
}

Node* new_ident(const char* s) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) die("malloc");
    n->type = NODE_IDENT;
    n->number = 0;
    strncpy(n->ident, s, sizeof(n->ident) - 1);
    n->ident[sizeof(n->ident) - 1] = '\0';
    n->op = 0;
    n->left = NULL;
    n->right = NULL;
    return n;
}

Node* new_op(char op, Node* l, Node* r) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) die("malloc");
    n->type = NODE_OP;
    n->number = 0;
    n->ident[0] = '\0';
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}

Node* clone_tree(const Node* n) {
    if (!n) return NULL;

    Node* c = (Node*)malloc(sizeof(Node));
    if (!c) die("malloc");

    *c = *n;
    c->left = clone_tree(n->left);
    c->right = clone_tree(n->right);
    return c;
}

void free_tree(Node* n) {
    if (!n) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

int is_number(Node* n) {
    return n && n->type == NODE_NUMBER;
}

int is_zero(Node* n) {
    return is_number(n) && n->number == 0;
}

void print_number(FILE* out, int x) {
    fprintf(out, "%d", x);
}

int tokenize(const char* s, Token tokens[], int* count) {
    int i = 0;
    int n = 0;

    while (s[i]) {
        if (isspace((unsigned char)s[i])) {
            i++;
            continue;
        }

        if (isdigit((unsigned char)s[i])) {
            int start = i;
            while (isdigit((unsigned char)s[i])) i++;

            if (n >= MAX_TOKENS) return 0;

            int len = i - start;
            if (len >= (int)sizeof(tokens[n].text)) return 0;

            strncpy(tokens[n].text, s + start, len);
            tokens[n].text[len] = '\0';
            tokens[n].type = TOK_NUMBER;
            tokens[n].number = atoi(tokens[n].text);
            n++;
            continue;
        }

        if (isalpha((unsigned char)s[i]) || s[i] == '_') {
            int start = i;
            while (isalnum((unsigned char)s[i]) || s[i] == '_') i++;

            if (n >= MAX_TOKENS) return 0;

            int len = i - start;
            if (len >= (int)sizeof(tokens[n].text)) return 0;

            strncpy(tokens[n].text, s + start, len);
            tokens[n].text[len] = '\0';
            tokens[n].type = TOK_IDENT;
            n++;
            continue;
        }

        if (strchr("+-*/()", s[i])) {
            if (n >= MAX_TOKENS) return 0;

            tokens[n].text[0] = s[i];
            tokens[n].text[1] = '\0';

            if (s[i] == '(') tokens[n].type = TOK_LPAREN;
            else if (s[i] == ')') tokens[n].type = TOK_RPAREN;
            else {
                tokens[n].type = TOK_OP;
                tokens[n].op = s[i];
            }

            n++;
            i++;
            continue;
        }

        return 0;
    }

    *count = n;
    return 1;
}

int precedence(char op) {
    switch (op) {
        case '~': return 3;
        case '*':
        case '/': return 2;
        case '+':
        case '-': return 1;
        default: return -1;
    }
}

int is_right_assoc(char op) {
    return op == '~';
}

int to_rpn(Token in[], int in_count, Token out[], int* out_count) {
    Token stack[MAX_TOKENS];
    int top = -1;
    int k = 0;
    int expect_operand = 1;

    for (int i = 0; i < in_count; i++) {
        Token t = in[i];

        if (t.type == TOK_NUMBER || t.type == TOK_IDENT) {
            if (k >= MAX_TOKENS) return 0;
            out[k++] = t;
            expect_operand = 0;
        }
        else if (t.type == TOK_OP) {
            char op = t.op;

            if (op == '-' && expect_operand) op = '~';

            while (top >= 0 && stack[top].type == TOK_OP) {
                char sop = stack[top].op;
                int p1 = precedence(op);
                int p2 = precedence(sop);

                if ((!is_right_assoc(op) && p1 <= p2) ||
                    ( is_right_assoc(op) && p1 <  p2)) {
                    if (k >= MAX_TOKENS) return 0;
                    out[k++] = stack[top--];
                } else {
                    break;
                }
            }

            t.op = op;
            stack[++top] = t;
            expect_operand = 1;
        }
        else if (t.type == TOK_LPAREN) {
            stack[++top] = t;
            expect_operand = 1;
        }
        else if (t.type == TOK_RPAREN) {
            int found = 0;
            while (top >= 0) {
                if (stack[top].type == TOK_LPAREN) {
                    found = 1;
                    top--;
                    break;
                }
                if (k >= MAX_TOKENS) return 0;
                out[k++] = stack[top--];
            }
            if (!found) return 0;
            expect_operand = 0;
        }
    }

    while (top >= 0) {
        if (stack[top].type == TOK_LPAREN || stack[top].type == TOK_RPAREN) {
            return 0;
        }
        if (k >= MAX_TOKENS) return 0;
        out[k++] = stack[top--];
    }

    *out_count = k;
    return 1;
}

Node* build_tree_from_rpn(Token rpn[], int count) {
    Node* st[MAX_TOKENS];
    int top = -1;

    for (int i = 0; i < count; i++) {
        Token t = rpn[i];

        if (t.type == TOK_NUMBER) {
            st[++top] = new_number(t.number);
        }
        else if (t.type == TOK_IDENT) {
            st[++top] = new_ident(t.text);
        }
        else if (t.type == TOK_OP) {
            if (t.op == '~') {
                if (top < 0) die("Некорректное выражение (унарный минус)");
                Node* a = st[top--];
                st[++top] = new_op('-', new_number(0), a);
            } else {
                if (top < 1) die("Некорректное выражение (бинарная операция)");
                Node* r = st[top--];
                Node* l = st[top--];
                st[++top] = new_op(t.op, l, r);
            }
        }
        else {
            die("Некорректный RPN");
        }
    }

    if (top != 0) die("Ошибка построения дерева");
    return st[top];
}

void print_tree(Node* n, int level) {
    if (!n) return;

    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    if (n->type == NODE_NUMBER) {
        printf("NUMBER(");
        print_number(stdout, n->number);
        printf(")\n");
    } else if (n->type == NODE_IDENT) {
        printf("IDENT(%s)\n", n->ident);
    } else {
        printf("OP(%c)\n", n->op);
        print_tree(n->left, level + 1);
        print_tree(n->right, level + 1);
    }
}

int node_precedence(Node* n) {
    if (!n) return 100;
    if (n->type != NODE_OP) return 100;
    return precedence(n->op);
}

void to_infix(Node* n, char* out, size_t cap);

void append_str(char* out, size_t cap, const char* s) {
    size_t cur = strlen(out);
    size_t add = strlen(s);
    if (cur + add + 1 >= cap) die("Слишком длинная строка результата");
    strcat(out, s);
}

void node_to_string(Node* n, char* buf, size_t cap) {
    buf[0] = '\0';

    if (n->type == NODE_NUMBER) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%d", n->number);
        append_str(buf, cap, tmp);
    }
    else if (n->type == NODE_IDENT) {
        append_str(buf, cap, n->ident);
    }
    else {
        to_infix(n, buf, cap);
    }
}

void to_infix(Node* n, char* out, size_t cap) {
    out[0] = '\0';
    if (!n) return;

    if (n->type == NODE_NUMBER || n->type == NODE_IDENT) {
        node_to_string(n, out, cap);
        return;
    }

    char L[MAX_TEXT], R[MAX_TEXT];
    node_to_string(n->left, L, sizeof(L));
    node_to_string(n->right, R, sizeof(R));

    int need_lp = 0;
    int need_rp = 0;

    if (n->left && n->left->type == NODE_OP) {
        if (node_precedence(n->left) < node_precedence(n)) need_lp = 1;
    }

    if (n->right && n->right->type == NODE_OP) {
        int pr = node_precedence(n->right);
        int pn = node_precedence(n);
        if (pr < pn) need_rp = 1;
        if ((n->op == '-' || n->op == '/') && pr == pn) need_rp = 1;
    }

    if (need_lp) append_str(out, cap, "(");
    append_str(out, cap, L);
    if (need_lp) append_str(out, cap, ")");

    char opbuf[4];
    snprintf(opbuf, sizeof(opbuf), " %c ", n->op);
    append_str(out, cap, opbuf);

    if (need_rp) append_str(out, cap, "(");
    append_str(out, cap, R);
    if (need_rp) append_str(out, cap, ")");
}

Node* simplify_once(Node* n, int* changed) {
    if (!n) return NULL;

    if (n->type != NODE_OP) {
        return clone_tree(n);
    }

    Node* L = simplify_once(n->left, changed);
    Node* R = simplify_once(n->right, changed);

    if (n->op == '-' && is_number(L) && is_number(R)) {
        int res = L->number - R->number;
        *changed = 1;
        free_tree(L);
        free_tree(R);
        return new_number(res);
    }

    if (n->op == '+') {
        if (is_zero(L)) {
            *changed = 1;
            free_tree(L);
            return R;
        }
        if (is_zero(R)) {
            *changed = 1;
            free_tree(R);
            return L;
        }
    }

    if (n->op == '+' && is_number(R) && R->number < 0) {
        int k = -R->number;
        *changed = 1;
        free_tree(R);
        return new_op('-', L, new_number(k));
    }

    if (n->op == '+' && is_number(L) && L->number < 0) {
        int k = -L->number;
        *changed = 1;
        free_tree(L);
        return new_op('-', R, new_number(k));
    }

    if (n->op == '-' && is_number(R)) {
        if (L->type == NODE_OP && L->op == '+' && is_number(L->right)) {
            Node* A = clone_tree(L->left);
            int c1 = L->right->number;
            int c2 = R->number;
            int diff = c1 - c2;

            free_tree(L);
            free_tree(R);
            *changed = 1;

            if (diff == 0) {
                return A;
            } else if (diff > 0) {
                return new_op('+', A, new_number(diff));
            } else {
                return new_op('-', A, new_number(-diff));
            }
        }
    }

    if (n->op == '-' && is_number(R)) {
        if (L->type == NODE_OP && L->op == '+' && is_number(L->left)) {
            int c1 = L->left->number;
            Node* A = clone_tree(L->right);
            int c2 = R->number;
            int diff = c1 - c2;

            free_tree(L);
            free_tree(R);
            *changed = 1;

            if (diff == 0) {
                return A;
            } else if (diff > 0) {
                return new_op('+', new_number(diff), A);
            } else {
                return new_op('-', A, new_number(-diff));
            }
        }
    }

    return new_op(n->op, L, R);
}

Node* simplify_full(Node* root) {
    Node* cur = clone_tree(root);

    while (1) {
        int changed = 0;
        Node* next = simplify_once(cur, &changed);
        free_tree(cur);
        cur = next;
        if (!changed) break;
    }

    return cur;
}

void process_expression(const char* expr) {
    Token tokens[MAX_TOKENS];
    Token rpn[MAX_TOKENS];
    int tcount = 0;
    int rcount = 0;

    if (!tokenize(expr, tokens, &tcount)) {
        die("Ошибка лексического анализа");
    }

    if (!to_rpn(tokens, tcount, rpn, &rcount)) {
        die("Ошибка преобразования в ОПЗ (алгоритм Дейкстры)");
    }

    Node* tree = build_tree_from_rpn(rpn, rcount);
    Node* simplified = simplify_full(tree);

    char src[MAX_TEXT], res[MAX_TEXT];
    to_infix(tree, src, sizeof(src));
    to_infix(simplified, res, sizeof(res));

    printf("Исходное выражение:\n%s\n\n", src);

    printf("Дерево исходного выражения:\n");
    print_tree(tree, 0);
    printf("\n");

    printf("Дерево после преобразований:\n");
    print_tree(simplified, 0);
    printf("\n");

    printf("Результат в текстовом виде:\n%s\n", res);

    free_tree(tree);
    free_tree(simplified);
}

int main(void) {
    char input[MAX_TEXT];

    printf("Введите выражение: ");

    if (!fgets(input, sizeof(input), stdin)) {
        die("Ошибка чтения строки");
    }

    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    process_expression(input);
    return 0;
}