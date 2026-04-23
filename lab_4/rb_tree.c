#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEY_LEN 7
#define BUF_SIZE 256

typedef enum { RED, BLACK } Color;

typedef struct Node {
    char key[KEY_LEN];
    double val;
    Color color;
    struct Node *left, *right, *parent;
} Node;

typedef struct {
    Node *root;
    Node *nil;
} Tree;

int valid_key(const char *k) {
    int len = strlen(k);
    if (len == 0 || len > 6) return 0;
    for (int i = 0; i < len; i++)
        if (!isalpha((unsigned char)k[i])) return 0;
    return 1;
}

Node *new_node(Tree *t, const char *k, double v) {
    Node *n = malloc(sizeof(Node));
    strcpy(n->key, k);
    n->val = v;
    n->color = RED;
    n->left = n->right = n->parent = t->nil;
    return n;
}

void init(Tree *t) {
    t->nil = malloc(sizeof(Node));
    t->nil->color = BLACK;
    t->root = t->nil;
}

void rotate_left(Tree *t, Node *x) {
    Node *y = x->right;
    x->right = y->left;
    if (y->left != t->nil) y->left->parent = x;
    y->parent = x->parent;

    if (x->parent == t->nil) t->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;

    y->left = x;
    x->parent = y;
}

void rotate_right(Tree *t, Node *y) {
    Node *x = y->left;
    y->left = x->right;
    if (x->right != t->nil) x->right->parent = y;
    x->parent = y->parent;

    if (y->parent == t->nil) t->root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;

    x->right = y;
    y->parent = x;
}

Node *find(Tree *t, const char *k) {
    Node *cur = t->root;
    while (cur != t->nil) {
        int c = strcmp(k, cur->key);
        if (c == 0) return cur;
        cur = (c < 0) ? cur->left : cur->right;
    }
    return t->nil;
}

void fix_insert(Tree *t, Node *z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node *y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotate_left(t, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_right(t, z->parent->parent);
            }
        } else {
            Node *y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(t, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_left(t, z->parent->parent);
            }
        }
    }
    t->root->color = BLACK;
}

int insert(Tree *t, const char *k, double v) {
    if (find(t, k) != t->nil) return 0;

    Node *z = new_node(t, k, v);
    Node *p = t->nil;
    Node *cur = t->root;

    while (cur != t->nil) {
        p = cur;
        if (strcmp(z->key, cur->key) < 0) cur = cur->left;
        else cur = cur->right;
    }

    z->parent = p;
    if (p == t->nil) t->root = z;
    else if (strcmp(z->key, p->key) < 0) p->left = z;
    else p->right = z;

    fix_insert(t, z);
    return 1;
}

Node *min_node(Tree *t, Node *x) {
    while (x->left != t->nil) x = x->left;
    return x;
}

void transplant(Tree *t, Node *u, Node *v) {
    if (u->parent == t->nil) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

void fix_delete(Tree *t, Node *x) {
    while (x != t->root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_left(t, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotate_right(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotate_left(t, x->parent);
                x = t->root;
            }
        } else {
            Node *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_right(t, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotate_left(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotate_right(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = BLACK;
}

int erase(Tree *t, const char *k) {
    Node *z = find(t, k);
    if (z == t->nil) return 0;

    Node *y = z;
    Node *x;
    Color orig = y->color;

    if (z->left == t->nil) {
        x = z->right;
        transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        transplant(t, z, z->left);
    } else {
        y = min_node(t, z->right);
        orig = y->color;
        x = y->right;

        if (y->parent == z) x->parent = y;
        else {
            transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    if (orig == BLACK) fix_delete(t, x);
    return 1;
}

void print_rec(Tree *t, Node *n, int d, FILE *out) {
    if (n == t->nil) return;
    print_rec(t, n->right, d + 1, out);
    for (int i = 0; i < d; i++) fprintf(out, "    ");
    fprintf(out, "%s(%.6g,%c)\n", n->key, n->val, n->color == RED ? 'R' : 'B');
    print_rec(t, n->left, d + 1, out);
}

void print_tree(Tree *t, FILE *out) {
    if (t->root == t->nil) fprintf(out, "<empty>\n");
    else print_rec(t, t->root, 0, out);
}

void process(Tree *t, char *line, FILE *out) {
    int cmd;
    char k[KEY_LEN];
    double v;

    fprintf(out, "%s\n", line);

    if (sscanf(line, "%d", &cmd) != 1) {
        fprintf(out, "ERROR\n\n");
        return;
    }

    if (cmd == 1) {
        if (sscanf(line, "%d %6s %lf", &cmd, k, &v) != 3 || !valid_key(k))
            fprintf(out, "ERROR\n\n");
        else if (insert(t, k, v))
            fprintf(out, "OK\n\n");
        else
            fprintf(out, "ERROR\n\n");
    } else if (cmd == 2) {
        if (sscanf(line, "%d %6s", &cmd, k) != 2 || !valid_key(k))
            fprintf(out, "ERROR\n\n");
        else if (erase(t, k))
            fprintf(out, "OK\n\n");
        else
            fprintf(out, "ERROR\n\n");
    } else if (cmd == 3) {
        print_tree(t, out);
        fprintf(out, "\n");
    } else if (cmd == 4) {
        if (sscanf(line, "%d %6s", &cmd, k) != 2 || !valid_key(k))
            fprintf(out, "ERROR\n\n");
        else {
            Node *n = find(t, k);
            if (n != t->nil) fprintf(out, "%.10g\n\n", n->val);
            else fprintf(out, "NOT FOUND\n\n");
        }
    } else {
        fprintf(out, "ERROR\n\n");
    }
}

int main() {
    FILE *in = fopen("input.txt", "r");
    FILE *out = fopen("output.txt", "w");

    Tree t;
    init(&t);

    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line)) process(&t, line, out);
    }

    fclose(in);
    fclose(out);
    return 0;
}