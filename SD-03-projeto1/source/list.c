/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include <stdlib.h>
#include <string.h>
#include "../include/list.h"
#include "../include/list-private.h"
#include "../include/data.h"

struct list_t *list_create() {
    struct list_t *list = malloc(sizeof(struct list_t));
    if (!list) return NULL;
    list->size = 0;
    list->head = NULL;
    return list; 
}

int list_destroy(struct list_t *list) {
    if (!list) return -1;
    struct car_t *cur = list->head;
    while (cur) {
        struct car_t *next = cur->next;
        if (cur->data) data_destroy(cur->data);
        free(cur);
        cur = next;
    }
    free(list);
    return 0;
}

int list_add(struct list_t *list, struct data_t *car) {
    if (!list || !car) return -1;
    struct car_t *node = malloc(sizeof(struct car_t));
    if (!node) return -1;
    node->data = car;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
    } else {
        struct car_t *cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }
    list->size++;
    return 0;
}

int list_remove_by_model(struct list_t *list, const char *modelo) {
    if (!list || !modelo) return -1;
    struct car_t *cur = list->head;
    struct car_t *prev = NULL;
    while (cur) {
        if (cur->data && cur->data->modelo && strcmp(cur->data->modelo, modelo) == 0) {
            if (prev) prev->next = cur->next;
            else list->head = cur->next;
            data_destroy(cur->data);
            free(cur);
            list->size--;
            return 0; /* found and removed */
        }
        prev = cur;
        cur = cur->next;
    }
    return 1; /* not found */
}

struct data_t *list_get_by_marca(struct list_t *list, enum marca_t marca) {
    if (!list) return NULL;
    struct car_t *cur = list->head;
    while (cur) {
        if (cur->data && cur->data->marca == marca)
            return cur->data;
        cur = cur->next;
    }
    return NULL;
}

struct data_t **list_get_by_year(struct list_t *list, int ano) {
    if (!list) return NULL;
    /* count matches */
    int count = 0;
    struct car_t *cur = list->head;
    while (cur) {
        if (cur->data && cur->data->ano == ano) count++;
        cur = cur->next;
    }
    /* allocate array (at least one slot for terminating NULL) */
    struct data_t **arr = malloc((count + 1) * sizeof(struct data_t *));
    if (!arr) return NULL;
    if (count == 0) {
        arr[0] = NULL;
        return arr;
    }
    int i = 0;
    cur = list->head;
    while (cur) {
        if (cur->data && cur->data->ano == ano) {
            arr[i++] = cur->data;
        }
        cur = cur->next;
    }
    arr[i] = NULL;
    return arr;
}

int list_order_by_year(struct list_t *list) {
    if (!list) return -1;
    if (!list->head || !list->head->next) return 0;

    /* simple bubble sort swapping data pointers */
    int swapped;
    do {
        swapped = 0;
        struct car_t *cur = list->head;
        while (cur->next) {
            if (cur->data && cur->next->data && cur->data->ano > cur->next->data->ano) {
                struct data_t *tmp = cur->data;
                cur->data = cur->next->data;
                cur->next->data = tmp;
                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
    return 0;
}

int list_size(struct list_t *list) {
    if (!list) return -1;
    return list->size;
}

char **list_get_model_list(struct list_t *list) {
    if (!list) return NULL;
    int n = list->size;
    /* allocate array of char* (n + 1 for NULL terminator) */
    char **models = malloc((n + 1) * sizeof(char *));
    if (!models) return NULL;
    struct car_t *cur = list->head;
    int i = 0;
    while (cur && i < n) {
        if (cur->data && cur->data->modelo) {
            models[i] = strdup(cur->data->modelo);
            if (!models[i]) {
                /* cleanup on failure */
                for (int j = 0; j < i; j++) free(models[j]);
                free(models);
                return NULL;
            }
        } else {
            models[i] = NULL;
        }
        i++;
        cur = cur->next;
    }
    models[i] = NULL;
    return models;
}

int list_free_model_list(char **models) {
    if (!models) return -1;
    char **p = models;
    while (*p) {
        free(*p);
        p++;
    }
    free(models);
    return 0;
}
