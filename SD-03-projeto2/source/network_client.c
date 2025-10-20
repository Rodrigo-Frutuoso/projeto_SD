/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_client.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int network_connect(struct rlist_t *rlist) { //SLIDES +11  TP4. Sockets
    if (rlist == NULL || rlist->server_address == NULL) {
        return -1;
    }

    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", rlist->server_port);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(rlist->server_address, port_str, &hints, &result) != 0) {
        return -1;
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(result);
        return -1;
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        close(sockfd);
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);
    rlist->sockfd = sockfd;
    return 0;
}

MessageT *network_send_receive(struct rlist_t *rlist, MessageT *msg) {
    if (rlist == NULL || msg == NULL || rlist->sockfd < 0) {
        return NULL;
    }

    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL) {
        return NULL;
    }

    message_t__pack(msg, buffer);

    uint32_t net_size = htonl(msg_size);
    if (write_all(rlist->sockfd, &net_size, sizeof(uint32_t)) != sizeof(uint32_t)) {
        free(buffer);
        return NULL;
    }

    if (write_all(rlist->sockfd, buffer, msg_size) != (int)msg_size) {
        free(buffer);
        return NULL;
    }
    free(buffer);

    if (read_all(rlist->sockfd, &net_size, sizeof(uint32_t)) != sizeof(uint32_t)) {
        return NULL;
    }
    uint32_t response_size = ntohl(net_size);

    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL) {
        return NULL;
    }

    if (read_all(rlist->sockfd, response_buffer, response_size) != (int)response_size) {
        free(response_buffer);
        return NULL;
    }

    MessageT *response = message_t__unpack(NULL, response_size, response_buffer);
    free(response_buffer);

    return response;
}

int network_close(struct rlist_t *rlist) {
    if (rlist == NULL || rlist->sockfd < 0) {
        return -1;
    }

    if (close(rlist->sockfd) < 0) {
        return -1;
    }

    rlist->sockfd = -1;
    return 0;
}
