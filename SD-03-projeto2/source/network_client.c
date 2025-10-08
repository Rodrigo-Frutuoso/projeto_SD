/**
 * Módulo de comunicação TCP do cliente.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "network_client.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"

int network_connect(struct rlist_t *rlist)
{
    if (rlist == NULL || rlist->server_address == NULL)
    {
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(rlist->server_port);

    // Tentar interpretar como IP direto
    if (inet_pton(AF_INET, rlist->server_address, &server_addr.sin_addr) <= 0)
    {
        // Se falhar, resolver hostname
        struct hostent *host = gethostbyname(rlist->server_address);
        if (host == NULL)
        {
            perror("gethostbyname");
            return -1;
        }
        memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
    }

    // Criar socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    // Conectar ao servidor
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    rlist->sockfd = sockfd;
    return 0;
}

MessageT *network_send_receive(struct rlist_t *rlist, MessageT *msg)
{
    if (rlist == NULL || msg == NULL || rlist->sockfd < 0)
    {
        return NULL;
    }

    // 1. Serializar a mensagem
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL)
    {
        return NULL;
    }

    message_t__pack(msg, buffer);

    // 2. Enviar tamanho da mensagem (4 bytes, network byte order)
    uint32_t net_size = htonl((uint32_t)msg_size);
    if (write_all(rlist->sockfd, &net_size, sizeof(net_size)) < 0)
    {
        free(buffer);
        return NULL;
    }

    // 3. Enviar mensagem serializada
    if (write_all(rlist->sockfd, buffer, msg_size) < 0)
    {
        free(buffer);
        return NULL;
    }

    free(buffer);

    // 4. Receber tamanho da resposta
    uint32_t response_net_size;
    if (read_all(rlist->sockfd, &response_net_size, sizeof(response_net_size)) < 0)
    {
        return NULL;
    }

    uint32_t response_size = ntohl(response_net_size);
    if (response_size == 0 || response_size > 10 * 1024 * 1024) // Limite de 10MB
    {
        return NULL;
    }

    // 5. Receber resposta serializada
    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL)
    {
        return NULL;
    }

    if (read_all(rlist->sockfd, response_buffer, response_size) < 0)
    {
        free(response_buffer);
        return NULL;
    }

    // 6. Desserializar resposta
    MessageT *response = message_t__unpack(NULL, response_size, response_buffer);
    free(response_buffer);

    return response;
}

int network_close(struct rlist_t *rlist)
{
    if (rlist == NULL || rlist->sockfd < 0)
    {
        return -1;
    }

    close(rlist->sockfd);
    rlist->sockfd = -1;
    return 0;
}
