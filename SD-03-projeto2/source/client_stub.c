/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "data.h"
#include "sdmessage.pb-c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct rlist_t *rlist_connect(char *address_port) {
    if (address_port == NULL) {
        return NULL;
    }

    char *colon = strchr(address_port, ':');
    if (colon == NULL) {
        return NULL;
    }

    struct rlist_t *rlist = malloc(sizeof(struct rlist_t));
    if (rlist == NULL) {
        return NULL;
    }

    size_t addr_len = colon - address_port;
    rlist->server_address = malloc(addr_len + 1);
    if (rlist->server_address == NULL) {
        free(rlist);
        return NULL;
    }

    strncpy(rlist->server_address, address_port, addr_len);
    rlist->server_address[addr_len] = '\0';

    rlist->server_port = atoi(colon + 1);
    if (rlist->server_port <= 0) {
        free(rlist->server_address);
        free(rlist);
        return NULL;
    }

    rlist->sockfd = -1;

    if (network_connect(rlist) < 0) {
        free(rlist->server_address);
        free(rlist);
        return NULL;
    }

    return rlist;
}

int rlist_disconnect(struct rlist_t *rlist) {
    if (rlist == NULL || network_close(rlist) < 0) {
        return -1;
    }

    free(rlist->server_address);
    free(rlist);
    return 0;
}

int rlist_add(struct rlist_t *rlist, struct data_t *car) {
    if (rlist == NULL || car == NULL) {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_ADD;
    msg.c_type = MESSAGE_T__C_TYPE__CT_DATA;

    Data data = DATA__INIT;
    data.ano = car->ano;
    data.preco = car->preco;
    data.marca = (Marca)car->marca;
    data.modelo = car->modelo;
    data.combustivel = (Combustivel)car->combustivel;
    msg.data = &data;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return -1;
    }

    int result = (response->opcode == MESSAGE_T__OPCODE__OP_ADD + 1 &&
                  response->c_type == MESSAGE_T__C_TYPE__CT_NONE) ? 0 : -1;

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_remove_by_model(struct rlist_t *rlist, const char *modelo) {
    if (rlist == NULL || modelo == NULL) {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_MODEL;
    msg.n_models = 1;
    char *models_array[1] = {(char *)modelo};
    msg.models = models_array;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return -1;
    }
    
    int result = response->result;
    message_t__free_unpacked(response, NULL);
    return result;
}

struct data_t *rlist_get_by_marca(struct rlist_t *rlist, enum marca_t marca) {
    if (rlist == NULL) {
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_MARCA;
    msg.result = marca;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return NULL;
    }

    struct data_t *car = NULL;
    if (response->opcode == MESSAGE_T__OPCODE__OP_GET + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_DATA &&
        response->data != NULL) {
        car = data_create(response->data->ano, response->data->preco,
                         (enum marca_t)response->data->marca, response->data->modelo,
                         (enum combustivel_t)response->data->combustivel);
    }

    message_t__free_unpacked(response, NULL);
    return car;
}

struct data_t **rlist_get_by_year(struct rlist_t *rlist, int ano) {
    if (rlist == NULL) {
        return NULL;
    }

    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_GETLISTBYTEAR;
    request.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    request.result = ano;

    MessageT *response = network_send_receive(rlist, &request);
    if (response == NULL) {
        return NULL;
    }

    if (response->opcode != MESSAGE_T__OPCODE__OP_GETLISTBYTEAR + 1 || 
        response->c_type != MESSAGE_T__C_TYPE__CT_LIST ||
        response->n_cars == 0 || response->cars == NULL) {
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    struct data_t **car_list = malloc((response->n_cars + 1) * sizeof(struct data_t *));
    if (car_list == NULL) {
        message_t__free_unpacked(response, NULL);
        return NULL;
    }

    size_t idx;
    for (idx = 0; idx < response->n_cars; idx++) {
        car_list[idx] = malloc(sizeof(struct data_t));
        if (car_list[idx] == NULL) {
            while (idx > 0) {
                free(car_list[--idx]->modelo);
                free(car_list[idx]);
            }
            free(car_list);
            message_t__free_unpacked(response, NULL);
            return NULL;
        }

        car_list[idx]->ano = response->cars[idx]->ano;
        car_list[idx]->preco = response->cars[idx]->preco;
        car_list[idx]->marca = (enum marca_t)response->cars[idx]->marca;
        car_list[idx]->modelo = strdup(response->cars[idx]->modelo);
        car_list[idx]->combustivel = (enum combustivel_t)response->cars[idx]->combustivel;
    }
    
    car_list[response->n_cars] = NULL;

    message_t__free_unpacked(response, NULL);
    return car_list;
}

int rlist_order_by_year(struct rlist_t *rlist) {
    if (rlist == NULL) {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETLISTBYTEAR;
    msg.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg.result = -1;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return -1;
    }

    int result = (response->opcode == MESSAGE_T__OPCODE__OP_GETLISTBYTEAR + 1 &&
                  response->c_type == MESSAGE_T__C_TYPE__CT_LIST) ? 0 : -1;

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_size(struct rlist_t *rlist) {
    if (rlist == NULL) {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return -1;
    }

    int result = (response->opcode == MESSAGE_T__OPCODE__OP_SIZE + 1 &&
                  response->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
                  ? response->result : -1;

    message_t__free_unpacked(response, NULL);
    return result;
}

char **rlist_get_model_list(struct rlist_t *rlist) {
    if (rlist == NULL) {
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETMODELS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL) {
        return NULL;
    }

    char **models = NULL;
    if (response->opcode == MESSAGE_T__OPCODE__OP_GETMODELS + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_MODEL) {
        models = malloc((response->n_models + 1) * sizeof(char *));
        if (models != NULL) {
            size_t i;
            for (i = 0; i < response->n_models; i++) {
                models[i] = strdup(response->models[i]);
                if (models[i] == NULL) {
                    while (i > 0) {
                        free(models[--i]);
                    }
                    free(models);
                    models = NULL;
                    break;
                }
            }
            if (models != NULL) {
                models[response->n_models] = NULL;
            }
        }
    }

    message_t__free_unpacked(response, NULL);
    return models;
}

int rlist_free_model_list(char **models) {
    if (models == NULL) {
        return -1;
    }

    for (int i = 0; models[i] != NULL; i++) {
        free(models[i]);
    }
    free(models);
    return 0;
}
