#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INF 1e9

typedef struct {
    int u;
    int v;
    int weight;
} Edge;

typedef struct AdjNode {
    int to;
    int weight;
    struct AdjNode* next;
} AdjNode;

void add_edge(AdjNode** adj_list, int u, int v, int weight) {
    AdjNode* new_node = (AdjNode*)malloc(sizeof(AdjNode));
    new_node->to = v;
    new_node->weight = weight;
    new_node->next = adj_list[u];
    adj_list[u] = new_node;
}

int main() {
    int num_vertices, num_edges;

    printf("Введите количество вершин и рёбер: ");
    if (scanf("%d %d", &num_vertices, &num_edges) != 2) {
        return 1;
    }

    Edge* edge_list = (Edge*)malloc(num_edges * sizeof(Edge));

    printf("Введите рёбра в формате (u v вес) [индексация вершин с 0 до %d]:\n", num_vertices - 1);
    for (int i = 0; i < num_edges; i++) {
        scanf("%d %d %d", &edge_list[i].u, &edge_list[i].v, &edge_list[i].weight);
    }

    AdjNode** adj_list = (AdjNode**)calloc(num_vertices, sizeof(AdjNode*));
    for (int i = 0; i < num_edges; i++) {
        add_edge(adj_list, edge_list[i].u, edge_list[i].v, edge_list[i].weight);
        add_edge(adj_list, edge_list[i].v, edge_list[i].u, edge_list[i].weight);
    }

    bool* in_mst = (bool*)calloc(num_vertices, sizeof(bool));
    int* min_weight = (int*)malloc(num_vertices * sizeof(int));
    int* parent = (int*)malloc(num_vertices * sizeof(int));

    for (int i = 0; i < num_vertices; i++) {
        min_weight[i] = INF;
        parent[i] = -1;
    }

    min_weight[0] = 0;
    int total_mst_weight = 0;

    printf("\nПошаговое добавление рёбер\n");

    for (int step = 0; step < num_vertices; step++) {
        int u = -1;
        int min_val = INF;
        for (int v = 0; v < num_vertices; v++) {
            if (!in_mst[v] && min_weight[v] < min_val) {
                min_val = min_weight[v];
                u = v;
            }
        }

        if (u == -1) {
            printf("Ошибка: Граф несвязный, невозможно построить единое остовное дерево.\n");
            break;
        }

        in_mst[u] = true;
        total_mst_weight += min_val;

        if (parent[u] != -1) {
            printf("Добавлено ребро: %d - %d (вес: %d)\n", parent[u], u, min_val);
        }

        AdjNode* curr = adj_list[u];
        while (curr != NULL) {
            int v = curr->to;
            int weight = curr->weight;

            if (!in_mst[v] && weight < min_weight[v]) {
                min_weight[v] = weight;
                parent[v] = u;
            }
            curr = curr->next;
        }
    }

    printf("\nОбщий вес минимального остовного дерева: %d\n", total_mst_weight);

    for (int i = 0; i < num_vertices; i++) {
        AdjNode* curr = adj_list[i];
        while (curr != NULL) {
            AdjNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(adj_list);
    free(edge_list);
    free(in_mst);
    free(min_weight);
    free(parent);

    return 0;
}