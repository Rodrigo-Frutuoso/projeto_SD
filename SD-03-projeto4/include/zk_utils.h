/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#ifndef _ZK_UTILS_H
#define _ZK_UTILS_H

#define THREADED
#include <zookeeper/zookeeper.h>

#define ZNODE_PATH_CHAIN "/chain"
#define ZNODE_PATH_NODE "/chain/node"

/**
 * Conecta ao ZooKeeper.
 * 
 * @param host String no formato "IP:porto" do servidor ZooKeeper
 * @param watcher Função callback para notificações (ou NULL)
 * @param context Contexto a passar para o watcher
 * @return Handle do ZooKeeper ou NULL em caso de erro
 */
zhandle_t *zk_connect(const char *host, watcher_fn watcher, void *context);

/**
 * Cria o ZNode /chain se ainda não existir.
 * 
 * @param zh Handle do ZooKeeper
 * @return 0 em sucesso, -1 em erro
 */
int zk_create_chain_znode(zhandle_t *zh);

/**
 * Cria um ZNode efémero sequencial filho de /chain.
 * 
 * @param zh Handle do ZooKeeper
 * @param data Dados a armazenar no ZNode (ex: "IP:porto")
 * @param path_buffer Buffer para armazenar o path completo criado
 * @param path_buffer_len Tamanho do buffer
 * @return 0 em sucesso, -1 em erro
 */
int zk_create_node_ephemeral_seq(zhandle_t *zh, const char *data, 
                                  char *path_buffer, int path_buffer_len);

/**
 * Obtém a lista de filhos de /chain.
 * 
 * @param zh Handle do ZooKeeper
 * @param watch Se 1, ativa watch para notificações de mudanças
 * @return String_vector com lista de filhos ou NULL em erro
 */
struct String_vector *zk_get_chain_children(zhandle_t *zh, int watch);

/**
 * Liberta memória alocada para String_vector.
 * 
 * @param strings String_vector a libertar
 */
void zk_free_string_vector(struct String_vector *strings);

/**
 * Obtém o conteúdo (data) de um ZNode.
 * 
 * @param zh Handle do ZooKeeper
 * @param path Path completo do ZNode
 * @param buffer Buffer para armazenar os dados
 * @param buffer_len Ponteiro para o tamanho do buffer (input/output)
 * @return 0 em sucesso, -1 em erro
 */
int zk_get_node_data(zhandle_t *zh, const char *path, char *buffer, int *buffer_len);

/**
 * Fecha a conexão com o ZooKeeper.
 * 
 * @param zh Handle do ZooKeeper
 */
void zk_disconnect(zhandle_t *zh);

/**
 * Obtém o ID do nó a partir do path completo.
 * Exemplo: "/chain/node0000000001" -> "node0000000001"
 * 
 * @param full_path Path completo do ZNode
 * @return Ponteiro para o ID do nó (dentro do full_path) ou NULL
 */
const char *zk_extract_node_id(const char *full_path);

/**
 * Compara dois IDs de nós para ordenação lexicográfica.
 * 
 * @param id1 Primeiro ID
 * @param id2 Segundo ID
 * @return <0 se id1<id2, 0 se iguais, >0 se id1>id2
 */
int zk_compare_node_ids(const char *id1, const char *id2);

#endif
