/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/serialization.h"
#include "../include/list-private.h"

int car_to_buffer(struct data_t *car, char **car_buf)
{
    if (car == NULL || car_buf == NULL || car->modelo == NULL) return -1;

    int modelo_len = (int)strlen(car->modelo);
    int total = sizeof(int) + sizeof(float) + sizeof(int) + sizeof(int) + sizeof(int) + modelo_len;

    char *buf = malloc(total);
    if (buf == NULL) return -1;

    char *p = buf;

    memcpy(p, &car->ano, sizeof(int)); p += sizeof(int);
    memcpy(p, &car->preco, sizeof(float)); p += sizeof(float);
    memcpy(p, &car->marca, sizeof(int)); p += sizeof(int);
    memcpy(p, &car->combustivel, sizeof(int)); p += sizeof(int);
    memcpy(p, &modelo_len, sizeof(int)); p += sizeof(int);
    memcpy(p, car->modelo, modelo_len);

    *car_buf = buf;
    return total;
}

struct data_t *buffer_to_car(char *car_buf)
{
    if (car_buf == NULL) return NULL;

    char *p = car_buf;
    int ano;
    float preco;
    int marca_i;
    int combustivel_i;
    int modelo_len;

    memcpy(&ano, p, sizeof(int)); p += sizeof(int);
    memcpy(&preco, p, sizeof(float)); p += sizeof(float);
    memcpy(&marca_i, p, sizeof(int)); p += sizeof(int);
    memcpy(&combustivel_i, p, sizeof(int)); p += sizeof(int);
    memcpy(&modelo_len, p, sizeof(int)); p += sizeof(int);

    if (modelo_len < 0) return NULL;

    char *modelo = malloc(modelo_len + 1);
    if (modelo == NULL) return NULL;
    if (modelo_len > 0) memcpy(modelo, p, modelo_len);
    modelo[modelo_len] = '\0';

    struct data_t *car = data_create(ano, preco, (enum marca_t)marca_i, modelo, (enum combustivel_t)combustivel_i);
    free(modelo);
    return car;
}

int car_list_to_buffer(struct list_t *list, char **list_buf)
{
    if (list == NULL || list_buf == NULL) return -1;

    int count = list->size;
    int total = sizeof(int);

    struct car_t *cur = list->head;
    while (cur) {
        if (cur->data == NULL || cur->data->modelo == NULL) return -1;
        int modelo_len = (int)strlen(cur->data->modelo);
        total += sizeof(int) + sizeof(float) + sizeof(int) + sizeof(int) + sizeof(int) + modelo_len;
        cur = cur->next;
    }

    char *buf = malloc(total);
    if (buf == NULL) return -1;

    char *p = buf;
    memcpy(p, &count, sizeof(int)); p += sizeof(int);

    cur = list->head;
    while (cur) {
        struct data_t *car = cur->data;
        int modelo_len = (int)strlen(car->modelo);

        memcpy(p, &car->ano, sizeof(int)); p += sizeof(int);
        memcpy(p, &car->preco, sizeof(float)); p += sizeof(float);
        memcpy(p, &car->marca, sizeof(int)); p += sizeof(int);
        memcpy(p, &car->combustivel, sizeof(int)); p += sizeof(int);
        memcpy(p, &modelo_len, sizeof(int)); p += sizeof(int);
        memcpy(p, car->modelo, modelo_len); p += modelo_len;

        cur = cur->next;
    }

    *list_buf = buf;
    return total;
}

struct list_t *buffer_to_car_list(char *list_buf)
{
    if (list_buf == NULL) return NULL;

    char *p = list_buf;
    int count;
    memcpy(&count, p, sizeof(int)); p += sizeof(int);
    if (count < 0) return NULL;

    struct list_t *list = list_create();
    if (list == NULL) return NULL;

    for (int i = 0; i < count; i++) {
        int ano;
        float preco;
        int marca_i;
        int combustivel_i;
        int modelo_len;

        memcpy(&ano, p, sizeof(int)); p += sizeof(int);
        memcpy(&preco, p, sizeof(float)); p += sizeof(float);
        memcpy(&marca_i, p, sizeof(int)); p += sizeof(int);
        memcpy(&combustivel_i, p, sizeof(int)); p += sizeof(int);
        memcpy(&modelo_len, p, sizeof(int)); p += sizeof(int);

        if (modelo_len < 0) { list_destroy(list); return NULL; }

        char *modelo = malloc(modelo_len + 1);
        if (modelo == NULL) { list_destroy(list); return NULL; }
        if (modelo_len > 0) memcpy(modelo, p, modelo_len);
        modelo[modelo_len] = '\0';
        p += modelo_len;

        struct data_t *car = data_create(ano, preco, (enum marca_t)marca_i, modelo, (enum combustivel_t)combustivel_i);
        free(modelo);
        if (car == NULL) { list_destroy(list); return NULL; }

        if (list_add(list, car) != 0) {
            data_destroy(car);
            list_destroy(list);
            return NULL;
        }
    }

    return list;
}