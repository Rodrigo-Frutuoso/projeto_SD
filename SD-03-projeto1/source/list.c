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

