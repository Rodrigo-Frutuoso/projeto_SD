/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "network_server-private.h"
#include "list.h"
#include "sdmessage.pb-c.h"
#include <stdlib.h>

int network_server_init(short port) {

}

int network_main_loop(int listening_socket, struct list_t *list) {

}

MessageT *network_receive(int client_socket) {

}

int network_send(int client_socket, MessageT *msg) {

}

int network_server_close(int socket) {

}

void network_server_request_shutdown(void) {

}
