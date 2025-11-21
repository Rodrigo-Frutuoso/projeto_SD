/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "chain_server.h"
#include "zk_utils.h"
#include "client_stub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/**
 * Inicializa a estrutura do servidor na cadeia.
 */
chain_server_t *chain_server_create(void) {
    chain_server_t *server = malloc(sizeof(chain_server_t));
    if (server == NULL) {
        fprintf(stderr, "Erro ao alocar memória para chain_server_t\n");
        return NULL;
    }

    // Inicializar campos
    server->zh = NULL;
    memset(server->node_id, 0, sizeof(server->node_id));
    memset(server->node_path, 0, sizeof(server->node_path));
    memset(server->successor_id, 0, sizeof(server->successor_id));
    server->next_server = NULL;
    server->is_tail = 1;  // Assume tail até encontrar sucessor
    server->is_head = 1;  // Assume head até encontrar antecessor
    
    // Inicializar mutex
    if (pthread_mutex_init(&server->replication_mutex, NULL) != 0) {
        fprintf(stderr, "Erro ao inicializar mutex de replicação\n");
        free(server);
        return NULL;
    }

    return server;
}

/**
 * Liberta recursos da estrutura do servidor.
 */
void chain_server_destroy(chain_server_t *server) {
    if (server == NULL) {
        return;
    }

    // Fechar conexão com sucessor
    if (server->next_server != NULL) {
        rlist_disconnect(server->next_server);
        server->next_server = NULL;
    }

    // Fechar conexão com ZooKeeper
    if (server->zh != NULL) {
        zk_disconnect(server->zh);
        server->zh = NULL;
    }

    // Destruir mutex
    pthread_mutex_destroy(&server->replication_mutex);

    free(server);
}

/**
 * Conecta o servidor ao ZooKeeper e regista-se na cadeia.
 */
int chain_server_connect_zk(chain_server_t *server, const char *zk_host, int server_port) {
    if (server == NULL || zk_host == NULL) {
        fprintf(stderr, "chain_server_connect_zk: argumentos inválidos\n");
        return -1;
    }

    // Conectar ao ZooKeeper (com callback para watch)
    server->zh = zk_connect(zk_host, chain_server_watcher, server);
    if (server->zh == NULL) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper\n");
        return -1;
    }

    // Criar ZNode /chain se não existir
    if (zk_create_chain_znode(server->zh) < 0) {
        fprintf(stderr, "Erro ao criar/verificar ZNode /chain\n");
        return -1;
    }

    // Obter IP local do servidor
    char server_ip[INET_ADDRSTRLEN];
    strcpy(server_ip, "127.0.0.1");  // Por simplicidade, usar localhost
    
    // Criar string "IP:porto" para armazenar no ZNode
    char node_data[256];
    snprintf(node_data, sizeof(node_data), "%s:%d", server_ip, server_port);

    // Criar ZNode efémero sequencial
    char path_buffer[512];
    if (zk_create_node_ephemeral_seq(server->zh, node_data, 
                                     path_buffer, sizeof(path_buffer)) < 0) {
        fprintf(stderr, "Erro ao criar ZNode efémero sequencial\n");
        return -1;
    }

    // Guardar path completo e extrair ID
    strncpy(server->node_path, path_buffer, sizeof(server->node_path) - 1);
    const char *node_id = zk_extract_node_id(path_buffer);
    if (node_id != NULL) {
        strncpy(server->node_id, node_id, sizeof(server->node_id) - 1);
        printf("Servidor registado no ZooKeeper com ID: %s\n", server->node_id);
    }

    return 0;
}

/**
 * Identifica e conecta-se ao servidor sucessor na cadeia.
 */
int chain_server_find_successor(chain_server_t *server) {
    if (server == NULL || server->zh == NULL) {
        fprintf(stderr, "chain_server_find_successor: servidor não inicializado\n");
        return -1;
    }

    // Obter lista de filhos de /chain com watch
    struct String_vector *children = zk_get_chain_children(server->zh, 1);
    if (children == NULL) {
        fprintf(stderr, "Erro ao obter filhos de /chain\n");
        return -1;
    }

    // Encontrar o sucessor (próximo ID após o nosso, em ordem lexicográfica)
    char *successor_id = NULL;
    for (int32_t i = 0; i < children->count; i++) {
        const char *child_id = children->data[i];
        
        // Comparar com nosso ID
        if (zk_compare_node_ids(child_id, server->node_id) > 0) {
            // Este nó vem depois do nosso
            if (successor_id == NULL || 
                zk_compare_node_ids(child_id, successor_id) < 0) {
                successor_id = (char *)child_id;
            }
        }
    }

    if (successor_id != NULL) {
        // Encontrou sucessor
        strncpy(server->successor_id, successor_id, sizeof(server->successor_id) - 1);
        server->is_tail = 0;
        
        // Obter IP:porto do sucessor
        char successor_path[512];
        snprintf(successor_path, sizeof(successor_path), "%s/%s", 
                ZNODE_PATH_CHAIN, successor_id);
        
        char successor_addr[256];
        int addr_len = sizeof(successor_addr);
        if (zk_get_node_data(server->zh, successor_path, successor_addr, &addr_len) == 0) {
            printf("Sucessor encontrado: %s (%s)\n", successor_id, successor_addr);
            
            // Conectar ao sucessor usando client_stub
            server->next_server = rlist_connect(successor_addr);
            if (server->next_server == NULL) {
                fprintf(stderr, "Erro ao conectar ao servidor sucessor\n");
                zk_free_string_vector(children);
                return -1;
            }
        }
    } else {
        // Não há sucessor, somos o tail
        server->is_tail = 1;
        server->successor_id[0] = '\0';
        printf("Este servidor é o TAIL da cadeia\n");
    }

    // Verificar se somos o head (não há ninguém antes de nós)
    int is_head = 1;
    for (int32_t i = 0; i < children->count; i++) {
        const char *child_id = children->data[i];
        if (zk_compare_node_ids(child_id, server->node_id) < 0) {
            is_head = 0;
            break;
        }
    }
    server->is_head = is_head;
    if (is_head) {
        printf("Este servidor é o HEAD da cadeia\n");
    }

    zk_free_string_vector(children);
    return 0;
}

/**
 * Identifica o servidor antecessor e sincroniza o estado da lista.
 */
int chain_server_sync_from_predecessor(chain_server_t *server, struct list_t *list) {
    if (server == NULL || server->zh == NULL || list == NULL) {
        fprintf(stderr, "chain_server_sync_from_predecessor: argumentos inválidos\n");
        return -1;
    }

    // Se somos o head, não há antecessor
    if (server->is_head) {
        printf("Servidor HEAD - sem sincronização necessária\n");
        return 0;
    }

    // Obter lista de filhos
    struct String_vector *children = zk_get_chain_children(server->zh, 0);
    if (children == NULL) {
        return -1;
    }

    // Encontrar antecessor (ID imediatamente antes do nosso)
    char *predecessor_id = NULL;
    for (int32_t i = 0; i < children->count; i++) {
        const char *child_id = children->data[i];
        
        if (zk_compare_node_ids(child_id, server->node_id) < 0) {
            if (predecessor_id == NULL || 
                zk_compare_node_ids(child_id, predecessor_id) > 0) {
                predecessor_id = (char *)child_id;
            }
        }
    }

    if (predecessor_id != NULL) {
        // Obter endereço do antecessor
        char pred_path[512];
        snprintf(pred_path, sizeof(pred_path), "%s/%s", ZNODE_PATH_CHAIN, predecessor_id);
        
        char pred_addr[256];
        int addr_len = sizeof(pred_addr);
        if (zk_get_node_data(server->zh, pred_path, pred_addr, &addr_len) == 0) {
            printf("Sincronizando estado com antecessor: %s (%s)\n", 
                   predecessor_id, pred_addr);
            
            // TODO: Implementar sincronização da lista
            // Por agora, apenas conecta e desconecta
            struct rlist_t *pred_conn = rlist_connect(pred_addr);
            if (pred_conn != NULL) {
                // Aqui seria feita a sincronização da lista
                // Será implementado na próxima fase
                rlist_disconnect(pred_conn);
            }
        }
    }

    zk_free_string_vector(children);
    return 0;
}

/**
 * Propaga uma operação para o servidor sucessor (se existir).
 */
int chain_server_propagate(chain_server_t *server, MessageT *msg) {
    if (server == NULL || msg == NULL) {
        return -1;
    }

    // Se somos tail, não há propagação
    if (server->is_tail || server->next_server == NULL) {
        return 0;
    }

    // TODO: Implementar propagação real
    // Será implementado na fase de propagação de operações
    
    return 0;
}

/**
 * Callback chamado quando há mudanças na cadeia (watch do ZooKeeper).
 */
void chain_server_watcher(zhandle_t *zh, int type, int state, 
                          const char *path, void *watcherCtx) {
    (void)zh;
    (void)state;
    (void)path;
    
    if (type == ZOO_CHILD_EVENT) {
        printf("Detetada mudança na cadeia de servidores\n");
        
        chain_server_t *server = (chain_server_t *)watcherCtx;
        if (server != NULL) {
            // Reprocessar a cadeia para encontrar novo sucessor
            // TODO: Implementar lógica de reconexão
            // Será implementado na fase de callback de watch
        }
    }
}
