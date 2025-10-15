/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "../include/serialization.h"
#include "../include/list-private.h"

int car_to_buffer(struct data_t *car, char **car_buf)
{
    if (car == NULL || car_buf == NULL || car->modelo == NULL) return -1;

    uint32_t modelo_len = (uint32_t)strlen(car->modelo);
    int total = (int)(sizeof(uint32_t) /*ano*/ + sizeof(float) /*preco*/ + sizeof(uint32_t) /*marca*/ + sizeof(uint32_t) /*comb*/ + sizeof(uint32_t) /*len*/ + modelo_len);

    char *buf = malloc(total);
    if (buf == NULL) return -1;

    char *p = buf;

    uint32_t n_ano = htonl((uint32_t)car->ano);
    memcpy(p, &n_ano, sizeof(uint32_t)); p += sizeof(uint32_t);
    memcpy(p, &car->preco, sizeof(float)); p += sizeof(float);
    uint32_t n_marca = htonl((uint32_t)car->marca);
    memcpy(p, &n_marca, sizeof(uint32_t)); p += sizeof(uint32_t);
    uint32_t n_comb = htonl((uint32_t)car->combustivel);
    memcpy(p, &n_comb, sizeof(uint32_t)); p += sizeof(uint32_t);
    uint32_t n_len = htonl(modelo_len);
    memcpy(p, &n_len, sizeof(uint32_t)); p += sizeof(uint32_t);
    memcpy(p, car->modelo, modelo_len);

    *car_buf = buf;
    return total;
}

struct data_t *buffer_to_car(char *car_buf)
{
    if (car_buf == NULL) return NULL;

    char *p = car_buf;
    uint32_t n_ano;
    float preco;
    uint32_t n_marca;
    uint32_t n_comb;
    uint32_t n_len;

    memcpy(&n_ano, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int ano = (int)ntohl(n_ano);
    memcpy(&preco, p, sizeof(float)); p += sizeof(float);
    memcpy(&n_marca, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int marca_i = (int)ntohl(n_marca);
    memcpy(&n_comb, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int combustivel_i = (int)ntohl(n_comb);
    memcpy(&n_len, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int modelo_len = (int)ntohl(n_len);

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

    uint32_t count = (uint32_t)list->size;
    int total = (int)sizeof(uint32_t);

    struct car_t *cur = list->head;
    while (cur) {
        if (cur->data == NULL || cur->data->modelo == NULL) return -1;
    uint32_t modelo_len = (uint32_t)strlen(cur->data->modelo);
    total += (int)(sizeof(uint32_t) + sizeof(float) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + modelo_len);
        cur = cur->next;
    }

    char *buf = malloc(total);
    if (buf == NULL) return -1;

    char *p = buf;
    uint32_t n_count = htonl(count);
    memcpy(p, &n_count, sizeof(uint32_t)); p += sizeof(uint32_t);

    cur = list->head;
    while (cur) {
    struct data_t *car = cur->data;
    uint32_t modelo_len = (uint32_t)strlen(car->modelo);

    uint32_t n_ano_i = htonl((uint32_t)car->ano);
    memcpy(p, &n_ano_i, sizeof(uint32_t)); p += sizeof(uint32_t);
    memcpy(p, &car->preco, sizeof(float)); p += sizeof(float);
    uint32_t n_marca_i = htonl((uint32_t)car->marca);
    memcpy(p, &n_marca_i, sizeof(uint32_t)); p += sizeof(uint32_t);
    uint32_t n_comb_i = htonl((uint32_t)car->combustivel);
    memcpy(p, &n_comb_i, sizeof(uint32_t)); p += sizeof(uint32_t);
    uint32_t n_len_i = htonl(modelo_len);
    memcpy(p, &n_len_i, sizeof(uint32_t)); p += sizeof(uint32_t);
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
    uint32_t n_count;
    memcpy(&n_count, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int count = (int)ntohl(n_count);
    if (count < 0) return NULL;

    struct list_t *list = list_create();
    if (list == NULL) return NULL;

    for (int i = 0; i < count; i++) {
    uint32_t n_ano;
    float preco;
    uint32_t n_marca;
    uint32_t n_comb;
    uint32_t n_len;

    memcpy(&n_ano, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int ano = (int)ntohl(n_ano);
    memcpy(&preco, p, sizeof(float)); p += sizeof(float);
    memcpy(&n_marca, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int marca_i = (int)ntohl(n_marca);
    memcpy(&n_comb, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int combustivel_i = (int)ntohl(n_comb);
    memcpy(&n_len, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    int modelo_len = (int)ntohl(n_len);

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
