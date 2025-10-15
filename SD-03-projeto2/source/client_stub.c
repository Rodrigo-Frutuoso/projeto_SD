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
#include <stdlib.h>

struct rlist_t *rlist_connect(char *address_port) {

}

int rlist_disconnect(struct rlist_t *rlist) {

}

int rlist_add(struct rlist_t *rlist, struct data_t *car) {

}

int rlist_remove_by_model(struct rlist_t *rlist, const char *modelo) {

}

struct data_t *rlist_get_by_marca(struct rlist_t *rlist, enum marca_t marca) {

}

struct data_t **rlist_get_by_year(struct rlist_t *rlist, int ano) {

}

int rlist_order_by_year(struct rlist_t *rlist) {

}

int rlist_size(struct rlist_t *rlist) {

}

char **rlist_get_model_list(struct rlist_t *rlist) {

}

int rlist_free_model_list(char **models) {

}
