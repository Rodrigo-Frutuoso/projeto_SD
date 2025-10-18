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

struct list_t *list_create()
{
    struct list_t *list = malloc(sizeof(struct list_t));
    if (list == NULL) return NULL;
    list->size = 0;
    list->head = NULL;
    return list;
}

int list_destroy(struct list_t *list)
{
    if (list == NULL) return -1;
    struct car_t *car = list->head;
    while(car){
        struct car_t *next_car = car->next;
        if(car->data) {
            data_destroy(car->data);
        }
        free(car);
        car = next_car;
    }
    free(list);
    return 0;
}

int list_add(struct list_t *list, struct data_t *car)
{
    if(list == NULL || car == NULL) return -1;
    struct car_t *car_added = malloc(sizeof(struct car_t));
    if(car_added == NULL) return -1;
    car_added->data = car;
    car_added->next = NULL;
    if(list->head == NULL){
        list->head = car_added;
    }else {
        struct car_t *car = list->head;
        while (car->next) {
            car = car->next;
        }
        car->next = car_added;
    }
    list->size++;
    return 0;
}

int list_remove_by_model(struct list_t *list, const char *modelo)
{
    if(list == NULL || modelo == NULL) return -1;
    struct car_t *car = list->head;
    struct car_t *car_backup = NULL;
        while (car) {
            if(car->data && car->data->modelo && strcmp(car->data->modelo, modelo) == 0) {
                if (car_backup) car_backup->next = car->next;
                else list->head = car->next;
                data_destroy(car->data);
                free(car);
                list->size--;
                return 0;
            }else{
                car_backup = car;
                car = car->next;
            }
        }
    return 1;
}

struct data_t *list_get_by_marca(struct list_t *list, enum marca_t marca)
{
    if(list == NULL) return NULL;
    struct car_t *car = list->head;
    while(car){
        if(car->data && car->data->marca == marca){
            return car->data;
        }
        car = car->next;
    }
    return NULL;
}

struct data_t **list_get_by_year(struct list_t *list, int ano)
{
    if (list == NULL) return NULL;
    int count = 0;
    struct car_t *car = list->head;
    while (car) {
        if (car->data && car->data->ano == ano) count++;
        car = car->next;
    }
    struct data_t **carros = malloc((count + 1) * sizeof(struct data_t *));
    if (carros == NULL) return NULL;
    if (count == 0) {
        carros[0] = NULL;
        return carros;
    }
    int i = 0;
    car = list->head;
    while (car) {
        if (car->data && car->data->ano == ano) {
            carros[i++] = car->data;
        }
        car = car->next;
    }
    carros[i] = NULL;
    return carros;
}

/* Ordena a lista de carros por ano de fabrico (crescente).
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_order_by_year(struct list_t *list)
{
    if (list == NULL) return -1;
    if (list->head == NULL || list->head->next == NULL) return 0;
    int swapped;
    struct car_t *ptr1;
    struct car_t *lptr = NULL;

    do {
        swapped = 0;
        ptr1 = list->head;

        while (ptr1->next != lptr) {
            if (ptr1->data && ptr1->next->data && ptr1->data->ano > ptr1->next->data->ano) {
                struct data_t *temp = ptr1->data;
                ptr1->data = ptr1->next->data;
                ptr1->next->data = temp;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);

    return 0;
}

int list_size(struct list_t *list)
{
    if (list == NULL) return -1;
    return list->size;
}

char **list_get_model_list(struct list_t *list)
{
    if (list == NULL) return NULL;
    char **modelos = malloc((list->size + 1) * sizeof(char *));
    if (modelos == NULL) return NULL;
    struct car_t *car = list->head;
    int i = 0;
    while (car) {
        if (car->data && car->data->modelo) {
            modelos[i] = strdup(car->data->modelo);
            i++;
        }
        car = car->next;
    }
    modelos[i] = NULL;
    return modelos;
}

int list_free_model_list(char **modelos)
{
    if (modelos == NULL) return -1;
    char **m = modelos;
    while (*m) {
        free(*m);
        m++;
    }
    free(modelos);
    return 0;
}

/* Devolve um array de ponteiros (terminado em NULL) para TODOS os carros.
 * Cada elemento do array aponta para dados internos da lista (não duplicados).
 * Cabe a quem chama libertar APENAS o array (free(result)), nunca os data_t*.
 * Retorna NULL em caso de erro.
 */
struct data_t **list_get_all(struct list_t *list)
{
    if (list == NULL) return NULL;

    // Aloca array com espaço para todos os carros + NULL no final
    struct data_t **carros = malloc((list->size + 1) * sizeof(struct data_t *));
    if (carros == NULL) return NULL;

    // Se a lista está vazia, retorna array com apenas NULL
    if (list->size == 0) {
        carros[0] = NULL;
        return carros;
    }

    // Percorre a lista e preenche o array com ponteiros para os dados
    int i = 0;
    struct car_t *car = list->head;
    while (car) {
        if (car->data) {
            carros[i++] = car->data;
        }
        car = car->next;
    }

    // Termina o array com NULL
    carros[i] = NULL;

    return carros;
}
