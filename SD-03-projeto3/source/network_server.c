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
#include <stdio.h>
#include <stdint.h>

static int server_sockfd = -1;
static int shutdown_requested = 0;

int network_server_init(short port) { //SLIDES +6  TP4. Sockets
    int sockfd;
    struct sockaddr_in server;
    int reuse = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
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

    while (!shutdown_requested && 
           (connsockfd = accept(listening_socket, (struct sockaddr *)&client, &size_client)) != -1) {
        printf("Connection established!\n");
        
        while (!shutdown_requested) {
            MessageT *msg = network_receive(connsockfd);
            if (msg == NULL) {
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
        printf("Connection closed!\n");
        
        if (!shutdown_requested) {
            printf("Server ready, waiting for connections\n");
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
