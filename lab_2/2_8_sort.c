#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 50
#define MAX_DATA 100
#define MIN_N 18

typedef struct {
    char key_str[MAX_STR];
    int key_int;
    char data[MAX_DATA];
} Item;

void clearBuffer() {
    while (getchar() != '\n') {
    }
}

int compare(Item a, Item b) {
    int cmp = strcmp(a.key_str, b.key_str);
    if (cmp != 0) {
        return cmp;
    }
    return a.key_int - b.key_int;
}

void printArray(Item arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%s %d | %s\n",
               arr[i].key_str,
               arr[i].key_int,
               arr[i].data);
    }
    printf("\n");
}

void selectionSortCount(Item arr[], int n) {
    Item *result = (Item*)malloc(n * sizeof(Item));

    if (result == NULL) {
        printf("Ошибка памяти\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++) {
            if (compare(arr[j], arr[i]) < 0) {
                count++;
            } else {
                if (compare(arr[j], arr[i]) == 0 && j < i) {
                    count++;
                }
            }
        }
        result[count] = arr[i];
    }

    for (int i = 0; i < n; i++) {
        arr[i] = result[i];
    }

    free(result);
}

int binarySearch(Item arr[], int n, Item key) {
    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = compare(arr[mid], key);

        if (cmp == 0) {
            return mid;
        } else {
            if (cmp < 0) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }
    return -1;
}

int main() {
    Item *arr;
    FILE *f = NULL;
    int n;
    int choice;

    do {
        printf("Введите размер таблицы (n >= %d): ", MIN_N);

        if (scanf("%d", &n) != 1) {
            printf("Ошибка ввода\n");
            clearBuffer();
            continue;
        }
        clearBuffer();

        if (n < MIN_N) {
            printf("Ошибка: n должно быть >= %d\n", MIN_N);
        }

    } while (n < MIN_N);
    arr = (Item*)malloc(n * sizeof(Item));

    if (arr == NULL) {
        printf("Ошибка выделения памяти\n");
        return 1;
    }

    printf("\nВыберите источник ввода:\n");
    printf("1 — консоль\n");
    printf("2 — файл\n");
    printf("Ваш выбор: ");

    if (scanf("%d", &choice) != 1) {
        printf("Ошибка ввода\n");
        free(arr);
        return 1;
    }
    clearBuffer();

    if (choice == 1) {
        f = stdin;
        printf("\nВведите %d строк (формат: строка число данные):\n", n);
    } else {
        if (choice == 2) {
            char filename[100];
            printf("Введите имя файла: ");
            scanf("%s", filename);
            clearBuffer();

            f = fopen(filename, "r");
            if (f == NULL) {
                printf("Не удалось открыть файл\n");
                free(arr);
                return 1;
            }
            printf("\nЧтение из файла...\n");
        } else {
            printf("Неверный выбор\n");
            free(arr);
            return 1;
        }
    }
    for (int i = 0; i < n; i++) {
        if (f == stdin) {
            printf("Элемент %d: ", i + 1);
        }
        if (fscanf(f, "%s %d %s",
                   arr[i].key_str,
                   &arr[i].key_int,
                   arr[i].data) != 3) {

            printf("Ошибка ввода на строке %d\n", i + 1);
            free(arr);
            return 1;
        }
    }
    if (f != stdin) {
        fclose(f);
    }

    printf("\nИсходный массив:\n");
    printArray(arr, n);

    selectionSortCount(arr, n);
    printf("Отсортированный массив:\n");
    printArray(arr, n);

    int k;
    printf("Сколько ключей искать? ");
    if (scanf("%d", &k) != 1 || k < 0) {
        printf("Ошибка ввода\n");
        free(arr);
        return 1;
    }
    clearBuffer();
    for (int i = 0; i < k; i++) {
        Item key;
        printf("Введите ключ (строка число): ");
        if (scanf("%s %d", key.key_str, &key.key_int) != 2) {
            printf("Ошибка ввода\n");
            clearBuffer();
            i--;
            continue;
        }
        clearBuffer();
        int pos = binarySearch(arr, n, key);
        if (pos != -1) {
            printf("Найден: %s %d | %s (позиция %d)\n",
                   arr[pos].key_str,
                   arr[pos].key_int,
                   arr[pos].data,
                   pos);
        } else {
            printf("Не найден\n");
        }
    }
    free(arr);
    return 0;
}
