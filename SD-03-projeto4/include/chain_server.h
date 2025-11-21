/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#ifndef _CHAIN_SERVER_H
#define _CHAIN_SERVER_H

#include "zk_utils.h"
#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include <pthread.h>

/**
 * Estrutura que mantém o estado do servidor na cadeia de replicação.
 */
typedef struct chain_server_t {
    // Conexão ao ZooKeeper
    zhandle_t *zh;

    // ID do próprio ZNode no ZooKeeper (ex: "node0000000001")
    char node_id[256];

    // Path completo do próprio ZNode (ex: "/chain/node0000000001")
    char node_path[512];

    // ID do ZNode do servidor sucessor na cadeia
    char successor_id[256];

    // Conexão ao servidor sucessor (reutiliza estrutura do cliente)
    struct rlist_t *next_server;

    // Mutex para proteger operações de replicação
    pthread_mutex_t replication_mutex;

    // Flag para indicar se é o servidor tail (não tem sucessor)
    int is_tail;

    // Flag para indicar se é o servidor head (não tem antecessor)
    int is_head;

} chain_server_t;

/**
 * Inicializa a estrutura do servidor na cadeia.
 *
 * @return Ponteiro para chain_server_t ou NULL em erro
 */
chain_server_t *chain_server_create(void);

/**
 * Liberta recursos da estrutura do servidor.
 *
 * @param server Estrutura a libertar
 */
void chain_server_destroy(chain_server_t *server);

/**
 * Conecta o servidor ao ZooKeeper e regista-se na cadeia.
 *
 * @param server Estrutura do servidor
 * @param zk_host IP:porto do ZooKeeper
 * @param server_port Porto do próprio servidor
 * @return 0 em sucesso, -1 em erro
 */
int chain_server_connect_zk(chain_server_t *server, const char *zk_host, int server_port);

/**
 * Identifica e conecta-se ao servidor sucessor na cadeia.
 * Deve ser chamada após obter a lista de filhos do ZooKeeper.
 *
 * @param server Estrutura do servidor
 * @return 0 em sucesso, -1 em erro
 */
int chain_server_find_successor(chain_server_t *server);

/**
 * Identifica o servidor antecessor e sincroniza o estado da lista.
 *
 * @param server Estrutura do servidor
 * @param list Lista local a preencher com dados do antecessor
 * @return 0 em sucesso, -1 em erro
 */
int chain_server_sync_from_predecessor(chain_server_t *server, struct list_t *list);

/**
 * Propaga uma operação para o servidor sucessor (se existir).
 * Esta função garante atomicidade com a operação local.
 *
 * @param server Estrutura do servidor
 * @param msg Mensagem a propagar
 * @return 0 em sucesso, -1 em erro
 */
int chain_server_propagate(chain_server_t *server, MessageT *msg);

/**
 * Callback chamado quando há mudanças na cadeia (watch do ZooKeeper).
 *
 * @param zh Handle do ZooKeeper
 * @param type Tipo de evento
 * @param state Estado da conexão
 * @param path Path do ZNode que mudou
 * @param watcherCtx Contexto (ponteiro para chain_server_t)
 */
void chain_server_watcher(zhandle_t *zh, int type, int state,
                          const char *path, void *watcherCtx);

#endif
