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
        case MESSAGE_T__OPCODE__OP_BAD:

        case MESSAGE_T__OPCODE__OP_ADD:
        
        case MESSAGE_T__OPCODE__OP_GET:

        case MESSAGE_T__OPCODE__OP_DEL:

        case MESSAGE_T__OPCODE__OP_SIZE:

        case MESSAGE_T__OPCODE__OP_GETMODELS:

        case MESSAGE_T__OPCODE__OP_GETLISTBYTEAR:

        case MESSAGE_T__OPCODE__OP_ORDER:

    }

}
