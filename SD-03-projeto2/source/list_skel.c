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

static Data *create_data_message(struct data_t *car) {
    if (car == NULL) {
        return NULL;
    }
    
    Data *data = malloc(sizeof(Data));
    if (data == NULL) {
        return NULL;
    }
    
    data__init(data);
    data->ano = car->ano;
    data->preco = car->preco;
    data->marca = (Marca)car->marca;
    data->modelo = strdup(car->modelo);
    data->combustivel = (Combustivel)car->combustivel;
    
    return data;
}

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
                        msg->data = create_data_message(car);
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
                                msg->cars[i] = create_data_message(cars[i]);
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
            if (msg->c_type != MESSAGE_T__C_TYPE__CT_MODEL ||
                msg->models == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            msg->result = list_remove_by_model(list, msg->models[0]);
            if ( msg->result == 0) {
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
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
            if (msg->c_type != MESSAGE_T__C_TYPE__CT_RESULT) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                return -1;
            }

            struct data_t **cars = NULL;
            int all = (msg->result == -1);
            
            if (all) {
                if (list_order_by_year(list) != 0) {
                    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    break;
                }
                cars = list_get_all(list);
            } else {
                cars = list_get_by_year(list, msg->result);
            }

            if (cars != NULL) {
                int count = 0;
                while (cars[count] != NULL) count++;

                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;
                msg->n_cars = count;
                msg->cars = malloc(count * sizeof(Data*));

                if (msg->cars != NULL) {
                    for (int i = 0; i < count; i++) {
                        msg->cars[i] = create_data_message(cars[i]);
                    }
                }
                free(cars);
            } else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }
            break;

        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }

    return 0;
}
