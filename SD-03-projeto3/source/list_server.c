/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "network_server-private.h"
#include "list_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void sigint_handler(int sig) {
    (void)sig;
    network_server_request_shutdown();
}

int main(int argc, char **argv) {
    struct list_t *list;
    int listening_socket;

    if (argc != 2) {
        fprintf(stderr, "Utilização: %s <porto>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 12345\n", argv[0]);
        return -1;
    }

    short port = (short) atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Erro: porta inválida '%s'\n", argv[1]);
        fprintf(stderr, "A porta deve ser um número entre 1 e 65535\n");
        return -1;
    }

    signal(SIGINT, sigint_handler);

    list = list_skel_init();
    if (list == NULL) {
        fprintf(stderr, "Erro ao inicializar a lista\n");
        return -1;
    }

    listening_socket = network_server_init(port);
    if (listening_socket < 0) {
        fprintf(stderr, "Error initializing network\n");
        list_skel_destroy(list);
        return -1;
    }

    if (network_main_loop(listening_socket, list) < 0) {
        fprintf(stderr, "Erro no loop principal do servidor\n");
    }

    while (1) {
        if (get_num_clientes_ativos() == 0)
            break;
        sleep(1);
    }

    if (listening_socket >= 0) {
        network_server_close(listening_socket);
    }

    if (list != NULL) {
        list_skel_destroy(list);
    }

    return 0;
}
