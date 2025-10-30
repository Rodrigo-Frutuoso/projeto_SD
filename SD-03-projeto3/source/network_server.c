/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "network_server-private.h"
#include "message-private.h"
#include "list.h"
#include "list_skel.h"
#include "sdmessage.pb-c.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>

#define MAX_CLIENTS 5
#define LOG_FILE "server.log"

/* Estrutura para passar argumentos às threads */
typedef struct {
    int client_socket;
    struct list_t *list;
    char client_addr[INET_ADDRSTRLEN];
    int client_port;
} thread_args_t;

/* Variáveis globais */
static int server_sockfd = -1;
static int shutdown_requested = 0;
static int num_clientes_ativos = 0;
static FILE *log_file = NULL;

/* Mutexes */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Função para obter timestamp atual em segundos */
static long get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

/* Função para converter opcode em string */
static const char* opcode_to_string(MessageT__Opcode opcode) {
    switch(opcode) {
        case MESSAGE_T__OPCODE__OP_ADD: return "OP_ADD";
        case MESSAGE_T__OPCODE__OP_GET: return "OP_GET";
        case MESSAGE_T__OPCODE__OP_DEL: return "OP_DEL";
        case MESSAGE_T__OPCODE__OP_SIZE: return "OP_SIZE";
        case MESSAGE_T__OPCODE__OP_GETMODELS: return "OP_GETMODELS";
        case MESSAGE_T__OPCODE__OP_GETLISTBYTEAR: return "OP_GETLISTBYTEAR";
        case MESSAGE_T__OPCODE__OP_ORDER: return "OP_ORDER";
        case MESSAGE_T__OPCODE__OP_ERROR: return "OP_ERROR";
        case MESSAGE_T__OPCODE__OP_BUSY: return "OP_BUSY";
        case MESSAGE_T__OPCODE__OP_READY: return "OP_READY";
        default: return "OP_UNKNOWN";
    }
}

/* Função para converter c_type em string */
static const char* ctype_to_string(MessageT__CType c_type) {
    switch(c_type) {
        case MESSAGE_T__C_TYPE__CT_DATA: return "CT_DATA";
        case MESSAGE_T__C_TYPE__CT_MARCA: return "CT_MARCA";
        case MESSAGE_T__C_TYPE__CT_YEAR: return "CT_YEAR";
        case MESSAGE_T__C_TYPE__CT_MODEL: return "CT_MODEL";
        case MESSAGE_T__C_TYPE__CT_RESULT: return "CT_RESULT";
        case MESSAGE_T__C_TYPE__CT_LIST: return "CT_LIST";
        case MESSAGE_T__C_TYPE__CT_NONE: return "CT_NONE";
        default: return "CT_UNKNOWN";
    }
}

/* Registar CONNECT no log */
static void log_connect(const char *client_addr, int client_port) {
    pthread_mutex_lock(&log_mutex);
    if (log_file != NULL) {
        long timestamp = get_timestamp();
        fprintf(log_file, "%ld %s:%d CONNECT\n", timestamp, client_addr, client_port);
        fflush(log_file);
    }
    pthread_mutex_unlock(&log_mutex);
}

/* Registar CLOSE no log */
static void log_close(const char *client_addr, int client_port) {
    pthread_mutex_lock(&log_mutex);
    if (log_file != NULL) {
        long timestamp = get_timestamp();
        fprintf(log_file, "%ld %s:%d CLOSE\n", timestamp, client_addr, client_port);
        fflush(log_file);
    }
    pthread_mutex_unlock(&log_mutex);
}

/* Registar REQUEST no log */
static void log_request(const char *client_addr, int client_port, MessageT *msg) {
    pthread_mutex_lock(&log_mutex);
    if (log_file != NULL && msg != NULL) {
        long timestamp = get_timestamp();
        fprintf(log_file, "%ld %s:%d REQUEST %s %s",
                timestamp, client_addr, client_port,
                opcode_to_string(msg->opcode),
                ctype_to_string(msg->c_type));

        /* Adicionar argumentos específicos conforme o tipo de pedido */
        if (msg->c_type == MESSAGE_T__C_TYPE__CT_DATA && msg->data != NULL) {
            /* Para OP_ADD: mostrar marca, modelo e ano */
            const char *marca_str = "";
            switch(msg->data->marca) {
                case MARCA__MARCA_TOYOTA: marca_str = "Toyota"; break;
                case MARCA__MARCA_BMW: marca_str = "BMW"; break;
                case MARCA__MARCA_RENAULT: marca_str = "Renault"; break;
                case MARCA__MARCA_AUDI: marca_str = "Audi"; break;
                case MARCA__MARCA_MERCEDES: marca_str = "Mercedes"; break;
                default: marca_str = "Unknown"; break;
            }
            fprintf(log_file, " %s %s %d",
                    marca_str,
                    msg->data->modelo ? msg->data->modelo : "",
                    msg->data->ano);
        } else if (msg->c_type == MESSAGE_T__C_TYPE__CT_MODEL && msg->n_models > 0 && msg->models != NULL) {
            fprintf(log_file, " %s", msg->models[0]);
        } else if (msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
            fprintf(log_file, " %d", msg->result);
        } else if (msg->c_type == MESSAGE_T__C_TYPE__CT_MARCA) {
            fprintf(log_file, " %d", msg->result);
        }

        fprintf(log_file, "\n");
        fflush(log_file);
    }
    pthread_mutex_unlock(&log_mutex);
}

/* Obter mutex da lista (para uso em list_skel.c) */
pthread_mutex_t* get_list_mutex() {
    return &list_mutex;
}

/* Função executada pelas threads secundárias para atender cada cliente */
void *client_handler(void *args) {
    thread_args_t *targs = (thread_args_t *)args;
    int connsockfd = targs->client_socket;
    struct list_t *list = targs->list;
    char client_addr[INET_ADDRSTRLEN];
    int client_port = targs->client_port;

    strcpy(client_addr, targs->client_addr);
    free(targs);

    /* Loop de atendimento do cliente */
    while (!shutdown_requested) {
        MessageT *msg = network_receive(connsockfd);
        if (msg == NULL) {
            break;
        }

        /* Registar REQUEST no log */
        log_request(client_addr, client_port, msg);

        /* Proteger acesso à lista com mutex */
        pthread_mutex_lock(&list_mutex);
        if (invoke(msg, list) < 0) {
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }
        pthread_mutex_unlock(&list_mutex);

        if (network_send(connsockfd, msg) < 0) {
            message_t__free_unpacked(msg, NULL);
            break;
        }

        message_t__free_unpacked(msg, NULL);
    }

    close(connsockfd);

    /* Registar CLOSE no log */
    log_close(client_addr, client_port);

    /* Decrementar número de clientes ativos */
    pthread_mutex_lock(&clients_mutex);
    num_clientes_ativos--;
    pthread_mutex_unlock(&clients_mutex);

    pthread_exit(NULL);
}

int network_server_init(short port) { //SLIDES +6  TP4. Sockets
    int sockfd;
    struct sockaddr_in server;
    int reuse = 1;

    /* Abrir ficheiro de log */
    log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Erro ao abrir ficheiro de log");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        close(sockfd);
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sockfd);
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    if (listen(sockfd, 0) < 0) {
        close(sockfd);
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    server_sockfd = sockfd;
    return sockfd;
}

int network_main_loop(int listening_socket, struct list_t *list) {
    if (listening_socket < 0 || list == NULL) {
        return -1;
    }

    struct sockaddr_in client;
    socklen_t size_client = sizeof(struct sockaddr_in);
    int connsockfd;

    printf("Server ready, waiting for connections\n");

    while (!shutdown_requested &&
           (connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client)) != -1) {

        /* Obter endereço e porto do cliente */
        char client_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr), client_addr, INET_ADDRSTRLEN);
        int client_port = ntohs(client.sin_port);

        /* Verificar número de clientes ativos */
        pthread_mutex_lock(&clients_mutex);
        int can_accept = (num_clientes_ativos < MAX_CLIENTS);
        pthread_mutex_unlock(&clients_mutex);

        if (can_accept) {
            /* Enviar mensagem OP_READY */
            MessageT msg = MESSAGE_T__INIT;
            msg.opcode = MESSAGE_T__OPCODE__OP_READY;
            msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

            if (network_send(connsockfd, &msg) < 0) {
                close(connsockfd);
                continue;
            }

            /* Incrementar número de clientes ativos */
            pthread_mutex_lock(&clients_mutex);
            num_clientes_ativos++;
            pthread_mutex_unlock(&clients_mutex);

            /* Registar CONNECT no log */
            log_connect(client_addr, client_port);

            printf("Connection established with %s:%d (Total clients: %d)\n",
                   client_addr, client_port, num_clientes_ativos);

            /* Criar estrutura de argumentos para a thread */
            thread_args_t *targs = malloc(sizeof(thread_args_t));
            if (targs == NULL) {
                close(connsockfd);
                pthread_mutex_lock(&clients_mutex);
                num_clientes_ativos--;
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }

            targs->client_socket = connsockfd;
            targs->list = list;
            strcpy(targs->client_addr, client_addr);
            targs->client_port = client_port;

            /* Criar thread para atender o cliente */
            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, client_handler, targs) != 0) {
                free(targs);
                close(connsockfd);
                pthread_mutex_lock(&clients_mutex);
                num_clientes_ativos--;
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }

            /* Detach thread para libertar recursos automaticamente */
            pthread_detach(thread_id);

        } else {
            /* Enviar mensagem OP_BUSY e rejeitar ligação */
            MessageT msg = MESSAGE_T__INIT;
            msg.opcode = MESSAGE_T__OPCODE__OP_BUSY;
            msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

            network_send(connsockfd, &msg);
            close(connsockfd);

            printf("Connection rejected from %s:%d - Server busy (max clients reached)\n",
                   client_addr, client_port);
        }
    }

    if (shutdown_requested) {
        return 0;
    }

    return -1;
}

MessageT *network_receive(int client_socket) {
    if (client_socket < 0) {
        return NULL;
    }

    uint16_t msg_size_net;

    int bytes_read = read_all(client_socket, &msg_size_net, sizeof(uint16_t));

    if (bytes_read != sizeof(uint16_t)) {
        return NULL;
    }

    uint16_t msg_size_host = ntohs(msg_size_net);
    uint32_t msg_size = msg_size_host;

    if (msg_size == 0 || msg_size > 65535) {
        return NULL;
    }

    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) {
        return NULL;
    }

    if (read_all(client_socket, buffer, msg_size) != (int)msg_size) {
        free(buffer);
        return NULL;
    }

    MessageT *msg = message_t__unpack(NULL, msg_size, buffer);
    free(buffer);

    return msg;
}

int network_send(int client_socket, MessageT *msg) {
    if (client_socket < 0 || msg == NULL) {
        return -1;
    }

    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) {
        return -1;
    }

    message_t__pack(msg, buffer);

    uint16_t size_host = (uint16_t)msg_size;
    uint16_t size_net = htons(size_host);

    if (write_all(client_socket, &size_net, sizeof(uint16_t)) != sizeof(uint16_t)) {
        free(buffer);
        return -1;
    }

    if (write_all(client_socket, buffer, msg_size) != (int)msg_size) {
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}

int network_server_close(int socket) {
    if (socket < 0) {
        return -1;
    }

    if (close(socket) < 0) {
        return -1;
    }

    /* Fechar ficheiro de log */
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }

    /* Destruir mutexes */
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&clients_mutex);
    pthread_mutex_destroy(&list_mutex);

    server_sockfd = -1;
    return 0;
}

void network_server_request_shutdown(void) {
    shutdown_requested = 1;

    if (server_sockfd >= 0) {
        close(server_sockfd);
        server_sockfd = -1;
    }
}
