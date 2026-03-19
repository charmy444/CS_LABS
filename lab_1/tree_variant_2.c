#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int data;
    struct node* left;
    struct node* right;
} node;

node* new_node(int data) {
    node *n = malloc(sizeof(node));
    n->data = data;
    n->left = n->right = NULL;
    return n;
}

void init_root(node **root) {
    *root = NULL;
}

void insert_node(node **n, int data) {
    if (*n == NULL) {
        *n = new_node(data);
    } else {
        if (data < (*n)->data) {
            insert_node(&((*n)->left), data);
        } else if (data > (*n)->data) {
            insert_node(&((*n)->right), data);
        }
    }
}

node* find_min(node *n) {
    while (n->left != NULL) {
        n = n->left;
    }
    return n;
}

int remove_node(node **n, int data) {
    if (*n == NULL) {
        return 0;
    }

    if (data < (*n)->data) {
        return remove_node(&((*n)->left), data);
    } else if (data > (*n)->data) {
        return remove_node(&((*n)->right), data);
    } else {
        node *temp = NULL;
        if ((*n)->left == NULL && (*n)->right == NULL) {
            free(*n);
            *n = NULL;
        }
        else if ((*n)->left == NULL) {
            temp = *n;
            *n = (*n)->right;
            free(temp);
        }
        else if ((*n)->right == NULL) {
            temp = *n;
            *n = (*n)->left;
            free(temp);
        }
        else {
            node *min_in_right = find_min((*n)->right);
            (*n)->data = min_in_right->data;
            remove_node(&((*n)->right), min_in_right->data);
        }
        return 1;
    }
}

void print_tree(node *n, int depth) {
    if (n == NULL) return;
    for (int i = 0; i < depth; i++)
        printf("\t");
    printf("%d\n", n->data);
    print_tree(n->left, depth + 1);
    print_tree(n->right, depth + 1);
}

int is_b_tree(node *n) {
    if (n == NULL) {
        return 1;
    }
    if ((n->left == NULL && n->right != NULL) ||
        (n->left != NULL && n->right == NULL)) {
        return 0;
    }
    return is_b_tree(n->left) && is_b_tree(n->right);
}

void free_tree(node *n) {
    if (n == NULL) {
        return;
    }
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

int main() {
    node *root;
    int operation, value;

    init_root(&root);

    while (1) {
        printf("\n Двоичное дерево\n\n"
               "[1] Создание нового узла\n"
               "[2] Удалить узел\n"
               "[3] Нарисовать дерево\n"
               "[4] Является ли дерево B деревом\n"
               "[0] Окончание программы\n");
        scanf("%d", &operation);
        getchar();

        switch (operation) {
            case 1:
                printf("Введи значение узла");
                scanf("%d", &value);
                insert_node(&root, value);
                printf("Узел вставлен\n");
                break;

            case 2:
                printf("Введи значение которое хочешь удалить");
                scanf("%d", &value);
                if (remove_node(&root, value) == 0)
                    printf("Узел не найден\n");
                else
                    printf("Узел удален\n");
                break;

            case 3:
                printf("Рисуем дерево\n");
                print_tree(root, 0);
                break;

            case 4:
                if (is_b_tree(root))
                    printf("Это B дерево\n");
                else
                    printf("Это НЕ B дерево\n");
                break;

            case 0:
                printf("Досвидание!\n");
                free_tree(root);
                return 0;

            default:
                printf("Несуществующая операция\n");
        }
        printf("\nНажми любую клавишу, очищаем буфер");
        getchar();
    }
}