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
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static int server_sockfd = -1;

int network_server_init(short port) { //SLIDES +6  TP4. Sockets
    int sockfd;
    struct sockaddr_in server;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    int option = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        close(sockfd);
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 0) < 0) {
        close(sockfd);
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

    while ((connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client)) != -1) {
        // Loop para processar múltiplas mensagens do mesmo cliente
        while (1) {
            MessageT *msg = network_receive(connsockfd);
            if (msg == NULL) {
                // Cliente desconectou ou erro na leitura
                break;
            }

            if (invoke(msg, list) < 0) {
                msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            if (network_send(connsockfd, msg) < 0) {
                message_t__free_unpacked(msg, NULL);
                break;
            }

            message_t__free_unpacked(msg, NULL);
        }

        close(connsockfd);
    }

    return -1;
}

MessageT *network_receive(int client_socket) {
    if (client_socket < 0) {
        return NULL;
    }

    uint32_t net_size;
    if (read_all(client_socket, &net_size, sizeof(uint32_t)) != sizeof(uint32_t)) {
        return NULL;
    }

    uint32_t msg_size = ntohl(net_size);
    if (msg_size == 0 || msg_size > 1048576) {
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

    uint32_t net_size = htonl(msg_size);
    if (write_all(client_socket, &net_size, sizeof(uint32_t)) != sizeof(uint32_t)) {
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

    server_sockfd = -1;
    return 0;
}

void network_server_request_shutdown(void) {
    if (server_sockfd >= 0) {
        close(server_sockfd);
        server_sockfd = -1;
    }
}
