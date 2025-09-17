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

/* Codigo do Simão */


struct list_t *list_create()
{
    struct list_t *list = malloc(sizeof(struct list_t));
    if (!list) return NULL;
    list->size = 0;
    list->head = NULL;
    return list;
}

int list_destroy(struct list_t *list)
{
    if (!list) return -1;
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
    if(!list || !car) return -1;
    struct car_t *car_added = malloc(sizeof(struct car_t));
    if(!car_added) return -1;
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
    if(!list || !modelo) return -1;
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

struct data_t *list_get_by_marca(struct list_t *list, enum marca_t marca){
    if(!list || !marca) return NULL;
    struct car_t *car = list->head;
    while(car){
        if(car->data && car->data->marca && (car->data->marca = marca)){
            return car->data;
        }
        car = car->next;
    }
    return NULL;
}


struct data_t **list_get_by_year(struct list_t *list, int ano)
{
    if (!list) return NULL;
    int count = 0;
    struct car_t *car = list->head;
    while (car) {
        if (car->data && car->data->ano == ano) count++;
        car = car->next;
    }
    struct data_t **carros = malloc((count + 1) * sizeof(struct data_t *));
    if (!carros) return NULL;
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
int list_order_by_year(struct list_t *list){


}

/* Retorna o número de carros na lista ou -1 em caso de erro.
 */
int list_size(struct list_t *list) {
    if (!list) return -1;
    return list->size;
}


/* Constrói um array de strings com os modelos dos carros na lista.
 * O último elemento do array é NULL.
 * Retorna o array ou NULL em caso de erro.
 */
char **list_get_model_list(struct list_t *list){


}

/* Liberta a memória ocupada pelo array de modelos.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_model_list(char **models)
{
    if (!models) return -1;
    char **p = models;
    while (*p) {
        free(*p);
        p++;
    }
    free(models);
    return 0;
}