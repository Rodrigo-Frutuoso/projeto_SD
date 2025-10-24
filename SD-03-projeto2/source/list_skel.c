/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "list_skel.h"
#include "list.h"
#include "data.h"
#include <string.h>
#include "sdmessage.pb-c.h"
#include <stdlib.h>

struct list_t *list_skel_init() {
    struct list_t *list = list_create();
    if(list == NULL){
        return NULL;
    }
    return list;
}

int list_skel_destroy(struct list_t *list) {
    if(list == NULL){
        return -1;
    }
    if(list_destroy(list) == 0){
        return 0;
    }
    return -1;
}

int invoke(MessageT *msg, struct list_t *list) {
    if(msg == NULL || list == NULL){
        return -1;
    }
    switch(msg->opcode){
        case MESSAGE_T__OPCODE__OP_ADD:
            {
                if (msg->c_type != MESSAGE_T__C_TYPE__CT_DATA || msg->data == NULL) {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }
                struct data_t *car = data_create(msg->data->ano, msg->data->preco,(enum marca_t)msg->data->marca,msg->data->modelo,(enum combustivel_t)msg->data->combustivel);
                if(car == NULL){
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }
                if (list_add(list, car) != 0) {
                    data_destroy(car);
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }
            break;

        case MESSAGE_T__OPCODE__OP_GET:
            {
                if (msg->c_type == MESSAGE_T__C_TYPE__CT_MARCA) {
                    struct data_t *car = list_get_by_marca(list, (enum marca_t)msg->result);
                    msg->opcode += 1;

                    if (car != NULL) {
                        msg->c_type = MESSAGE_T__C_TYPE__CT_DATA;
                        msg->data = malloc(sizeof(Data));
                        if (msg->data != NULL) {
                            data__init(msg->data);
                            msg->data->ano = car->ano;
                            msg->data->preco = car->preco;
                            msg->data->marca = (Marca)car->marca;
                            msg->data->modelo = strdup(car->modelo);
                            msg->data->combustivel = (Combustivel)car->combustivel;
                        }
                    } else {
                        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                        msg->data = NULL;
                    }
                }
                else if (msg->c_type == MESSAGE_T__C_TYPE__CT_YEAR) {
                    int ano = msg->result;
                    struct data_t **cars = list_get_by_year(list, ano);

                    if (cars != NULL) {
                        int count = 0;
                        while (cars[count] != NULL) count++;

                        msg->opcode += 1;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;
                        msg->n_cars = count;
                        msg->cars = malloc(count * sizeof(Data*));

                        if (msg->cars != NULL) {
                            for (int i = 0; i < count; i++) {
                                msg->cars[i] = malloc(sizeof(Data));
                                if (msg->cars[i] != NULL) {
                                    data__init(msg->cars[i]);
                                    msg->cars[i]->ano = cars[i]->ano;
                                    msg->cars[i]->preco = cars[i]->preco;
                                    msg->cars[i]->marca = (Marca)cars[i]->marca;
                                    msg->cars[i]->modelo = strdup(cars[i]->modelo);
                                    msg->cars[i]->combustivel = (Combustivel)cars[i]->combustivel;
                                }
                            }
                        }
                        free(cars);
                    } else {
                        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    }
                }
                else {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            }
            break;

        case MESSAGE_T__OPCODE__OP_DEL:
            {
                if (msg->n_models == 0 || msg->models == NULL || msg->models[0] == NULL) {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }

                int res = list_remove_by_model(list, msg->models[0]);
                if (res == 0) {
                    msg->opcode += 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    msg->result = 0;
                } else if (res == 1) {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                } else {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            }
            break;
        case MESSAGE_T__OPCODE__OP_SIZE:
            {
            int size = list_size(list);
            if (size >= 0) {
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
                msg->result = size;
            }
            else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            break;
            }
        case MESSAGE_T__OPCODE__OP_GETMODELS:
            {
                char **models = list_get_model_list(list);

                if (models != NULL) {
                    int count = 0;
                    while (models[count] != NULL) count++;

                    msg->opcode += 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_MODEL;
                    msg->n_models = count;
                    msg->models = malloc(count * sizeof(char*));

                    if (msg->models != NULL) {
                        for (int i = 0; i < count; i++) {
                            msg->models[i] = strdup(models[i]);
                        }
                    }
                    list_free_model_list(models);
                } else {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            }
            break;

        case MESSAGE_T__OPCODE__OP_GETLISTBYTEAR:
            {
                if (list_order_by_year(list) != 0) {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }

                struct data_t **all_cars = list_get_all(list);

                if (all_cars != NULL) {
                    int count = 0;
                    while (all_cars[count] != NULL) count++;

                    msg->opcode += 1;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;
                    msg->n_cars = count;
                    msg->cars = malloc(count * sizeof(Data*));

                    if (msg->cars != NULL) {
                        for (int i = 0; i < count; i++) {
                            msg->cars[i] = malloc(sizeof(Data));
                            if (msg->cars[i] != NULL) {
                                data__init(msg->cars[i]);
                                msg->cars[i]->ano = all_cars[i]->ano;
                                msg->cars[i]->preco = all_cars[i]->preco;
                                msg->cars[i]->marca = (Marca)all_cars[i]->marca;
                                msg->cars[i]->modelo = strdup(all_cars[i]->modelo);
                                msg->cars[i]->combustivel = (Combustivel)all_cars[i]->combustivel;
                            }
                        }
                    }
                    free(all_cars);
                } else {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                }
            }
            break;

        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }

    return 0;
}
