/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "zk_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define ZK_TIMEOUT 2000  // Timeout em milissegundos

/**
 * Conecta ao ZooKeeper.
 */
zhandle_t *zk_connect(const char *host, watcher_fn watcher, void *context) {
    if (host == NULL) {
        fprintf(stderr, "zk_connect: host é NULL\n");
        return NULL;
    }

    zhandle_t *zh = zookeeper_init(host, watcher, ZK_TIMEOUT, 0, context, 0);
    
    if (zh == NULL) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper em %s: %s\n", 
                host, strerror(errno));
        return NULL;
    }

    // Esperar pela conexão
    int state;
    int max_attempts = 10;
    for (int i = 0; i < max_attempts; i++) {
        state = zoo_state(zh);
        if (state == ZOO_CONNECTED_STATE) {
            printf("Conectado ao ZooKeeper em %s\n", host);
            return zh;
        }
        sleep(1);
    }

    fprintf(stderr, "Timeout ao conectar ao ZooKeeper\n");
    zookeeper_close(zh);
    return NULL;
}

/**
 * Cria o ZNode /chain se ainda não existir.
 */
int zk_create_chain_znode(zhandle_t *zh) {
    if (zh == NULL) {
        fprintf(stderr, "zk_create_chain_znode: zh é NULL\n");
        return -1;
    }

    // Tentar criar o ZNode /chain (persistente, não efémero)
    int rc = zoo_create(zh, ZNODE_PATH_CHAIN, NULL, -1,
                       &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);

    if (rc == ZOK) {
        printf("ZNode %s criado com sucesso\n", ZNODE_PATH_CHAIN);
        return 0;
    } else if (rc == ZNODEEXISTS) {
        // ZNode já existe, não é erro
        printf("ZNode %s já existe\n", ZNODE_PATH_CHAIN);
        return 0;
    } else {
        fprintf(stderr, "Erro ao criar ZNode %s: %s\n", 
                ZNODE_PATH_CHAIN, zerror(rc));
        return -1;
    }
}

/**
 * Cria um ZNode efémero sequencial filho de /chain.
 */
int zk_create_node_ephemeral_seq(zhandle_t *zh, const char *data, 
                                  char *path_buffer, int path_buffer_len) {
    if (zh == NULL || data == NULL || path_buffer == NULL) {
        fprintf(stderr, "zk_create_node_ephemeral_seq: argumentos inválidos\n");
        return -1;
    }

    // ZOO_EPHEMERAL | ZOO_SEQUENCE = efémero e sequencial
    int rc = zoo_create(zh, ZNODE_PATH_NODE, data, strlen(data),
                       &ZOO_OPEN_ACL_UNSAFE, 
                       ZOO_EPHEMERAL | ZOO_SEQUENCE,
                       path_buffer, path_buffer_len);

    if (rc == ZOK) {
        printf("ZNode efémero sequencial criado: %s\n", path_buffer);
        return 0;
    } else {
        fprintf(stderr, "Erro ao criar ZNode efémero sequencial: %s\n", 
                zerror(rc));
        return -1;
    }
}

/**
 * Obtém a lista de filhos de /chain.
 */
struct String_vector *zk_get_chain_children(zhandle_t *zh, int watch) {
    if (zh == NULL) {
        fprintf(stderr, "zk_get_chain_children: zh é NULL\n");
        return NULL;
    }

    struct String_vector *strings = malloc(sizeof(struct String_vector));
    if (strings == NULL) {
        fprintf(stderr, "Erro ao alocar memória para String_vector\n");
        return NULL;
    }

    int rc = zoo_get_children(zh, ZNODE_PATH_CHAIN, watch, strings);

    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de %s: %s\n", 
                ZNODE_PATH_CHAIN, zerror(rc));
        free(strings);
        return NULL;
    }

    return strings;
}

/**
 * Liberta memória alocada para String_vector.
 */
void zk_free_string_vector(struct String_vector *strings) {
    if (strings == NULL) {
        return;
    }

    // Libertar cada string
    if (strings->data != NULL) {
        for (int32_t i = 0; i < strings->count; i++) {
            if (strings->data[i] != NULL) {
                free(strings->data[i]);
            }
        }
        free(strings->data);
    }

    free(strings);
}

/**
 * Obtém o conteúdo (data) de um ZNode.
 */
int zk_get_node_data(zhandle_t *zh, const char *path, char *buffer, int *buffer_len) {
    if (zh == NULL || path == NULL || buffer == NULL || buffer_len == NULL) {
        fprintf(stderr, "zk_get_node_data: argumentos inválidos\n");
        return -1;
    }

    int rc = zoo_get(zh, path, 0, buffer, buffer_len, NULL);

    if (rc != ZOK) {
        fprintf(stderr, "Erro ao obter dados de %s: %s\n", path, zerror(rc));
        return -1;
    }

    // Adicionar terminador nulo
    if (*buffer_len < 256) {
        buffer[*buffer_len] = '\0';
    }

    return 0;
}

/**
 * Fecha a conexão com o ZooKeeper.
 */
void zk_disconnect(zhandle_t *zh) {
    if (zh != NULL) {
        printf("A fechar conexão com ZooKeeper...\n");
        zookeeper_close(zh);
    }
}

/**
 * Obtém o ID do nó a partir do path completo.
 */
const char *zk_extract_node_id(const char *full_path) {
    if (full_path == NULL) {
        return NULL;
    }

    // Procurar última '/'
    const char *last_slash = strrchr(full_path, '/');
    
    if (last_slash == NULL) {
        return full_path;
    }

    return last_slash + 1;
}

/**
 * Compara dois IDs de nós para ordenação lexicográfica.
 */
int zk_compare_node_ids(const char *id1, const char *id2) {
    if (id1 == NULL || id2 == NULL) {
        return 0;
    }

    return strcmp(id1, id2);
}
