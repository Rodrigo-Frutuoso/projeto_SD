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
            if (msg->data == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                break;
            }
            struct data_t *car = data_create(msg->data->ano, msg->data->preco,(enum marca_t)msg->data->marca,msg->data->modelo,(enumcombustivel_msg->data->combustivel));
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
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = 0;
            break;

        case MESSAGE_T__OPCODE__OP_GET:

            if (msg->data == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                break;
            }

            struct data_t *car = list_get_by_marca(list, (enum marca_t)msg->data->marca);    
            msg->opcode += 1;

            if (car != NULL) {
                msg->c_type = MESSAGE_T__C_TYPE__CT_DATA;
                msg->data->ano = car->ano;
                msg->data->preco = car->preco;
                msg->data->marca = (Marca)car->marca;
                msg->data->modelo = strdup(car->modelo);           
                msg->data->combustivel = (Combustivel)car->combustivel;
                data_destroy(car);
            }

            else {
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;       
            }

            break;

        case MESSAGE_T__OPCODE__OP_DEL:
            if (msg->data == NULL || msg->data->modelo == NULL) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                break;
            }

            if (list_remove_by_model(list, msg->data->modelo); == 0 ||list_remove_by_model(list, msg->data->modelo); == 1) 
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            } 

            else {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            break;

        case MESSAGE_T__OPCODE__OP_SIZE:

            break;
        case MESSAGE_T__OPCODE__OP_GETMODELS:

            break;
        case MESSAGE_T__OPCODE__OP_GETLISTBYTEAR:

            break;
        case MESSAGE_T__OPCODE__OP_ORDER:

            break;
        default:
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }

}
