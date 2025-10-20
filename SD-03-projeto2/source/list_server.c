/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "list_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char **argv) {
    struct list_t *list;
    int listening_socket;
    short port;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <porto>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 12345\n", argv[0]);
        return -1;
    }

    port = (short) atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Erro: porta inválida '%s'\n", argv[1]);
        fprintf(stderr, "A porta deve ser um número entre 1 e 65535\n");
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    list = list_skel_init();
    if (list == NULL) {
        fprintf(stderr, "Erro: falha ao inicializar a lista de carros\n");
        return -1;
    }

    printf("✓ Lista de carros inicializada\n");

    listening_socket = network_server_init(port);
    if (listening_socket < 0) {
        fprintf(stderr, "Erro: falha ao inicializar servidor na porta %d\n", port);
        list_skel_destroy(list);
        return -1;
    }

    printf("✓ Servidor a ouvir na porta %d\n", port);
    printf("✓ Aguardando conexões de clientes...\n");
    printf("  (Prima Ctrl+C para terminar)\n\n");

    if (network_main_loop(listening_socket, list) < 0) {
        fprintf(stderr, "Erro: falha no loop principal do servidor\n");
    }

    printf("\nEncerrando servidor...\n");

    network_server_close(listening_socket);
    printf("✓ Socket fechado\n");

    list_skel_destroy(list);
    printf("✓ Lista destruída e memória libertada\n");

    printf("Servidor terminado.\n");

    return 0;
}
