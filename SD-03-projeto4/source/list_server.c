/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "network_server-private.h"
#include "list_skel.h"
#include "chain_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Variável global para o chain_server (necessária para sigint_handler)
static chain_server_t *global_chain_server = NULL;

static void sigint_handler(int sig) {
    (void)sig;
    network_server_request_shutdown();
}

int main(int argc, char **argv) {
    struct list_t *list;
    int listening_socket;
    chain_server_t *chain_server = NULL;

    if (argc != 3) {
        fprintf(stderr, "Utilização: %s <porto_servidor> <IP:porto_ZooKeeper>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 12345 localhost:2181\n", argv[0]);
        return -1;
    }

    short port = (short) atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Erro: porta inválida '%s'\n", argv[1]);
        fprintf(stderr, "A porta deve ser um número entre 1 e 65535\n");
        return -1;
    }

    const char *zk_host = argv[2];

    signal(SIGINT, sigint_handler);

    // Inicializar lista
    list = list_skel_init();
    if (list == NULL) {
        fprintf(stderr, "Erro ao inicializar a lista\n");
        return -1;
    }

    // Criar estrutura do servidor na cadeia
    chain_server = chain_server_create();
    if (chain_server == NULL) {
        fprintf(stderr, "Erro ao criar estrutura chain_server\n");
        list_skel_destroy(list);
        return -1;
    }
    global_chain_server = chain_server;

    // Conectar ao ZooKeeper e registar servidor na cadeia
    printf("\n=== Fase 4: Chain Replication ===\n");
    printf("Conectando ao ZooKeeper: %s\n", zk_host);
    if (chain_server_connect_zk(chain_server, zk_host, port) < 0) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper\n");
        chain_server_destroy(chain_server);
        list_skel_destroy(list);
        return -1;
    }

    // Identificar sucessor na cadeia
    printf("\nIdentificando posição na cadeia de replicação...\n");
    if (chain_server_find_successor(chain_server) < 0) {
        fprintf(stderr, "Erro ao identificar sucessor\n");
        chain_server_destroy(chain_server);
        list_skel_destroy(list);
        return -1;
    }

    // Sincronizar estado com antecessor (se não for HEAD)
    if (!chain_server->is_head) {
        printf("\nSincronizando estado com servidor antecessor...\n");
        if (chain_server_sync_from_predecessor(chain_server, list) < 0) {
            fprintf(stderr, "Aviso: Erro ao sincronizar com antecessor\n");
            // Não é erro fatal, continuar
        }
    }

    printf("\n=== Servidor pronto ===\n");
    printf("Papel na cadeia: %s%s\n",
           chain_server->is_head ? "HEAD" : "",
           chain_server->is_tail ? (chain_server->is_head ? " e TAIL" : "TAIL") : "");
    printf("Aguardando conexões de clientes na porta %d...\n\n", port);

    // Inicializar socket de escuta
    listening_socket = network_server_init(port);
    if (listening_socket < 0) {
        fprintf(stderr, "Error initializing network\n");
        chain_server_destroy(chain_server);
        list_skel_destroy(list);
        return -1;
    }

    // Loop principal do servidor
    if (network_main_loop(listening_socket, list) < 0) {
        fprintf(stderr, "Erro no loop principal do servidor\n");
    }

    // Aguardar término de clientes ativos
    while (1) {
        if (get_num_clientes_ativos() == 0)
            break;
        sleep(1);
    }

    // Cleanup
    if (listening_socket >= 0) {
        network_server_close(listening_socket);
    }

    if (chain_server != NULL) {
        chain_server_destroy(chain_server);
    }

    if (list != NULL) {
        list_skel_destroy(list);
    }

    printf("\nServidor encerrado.\n");
    return 0;
}
