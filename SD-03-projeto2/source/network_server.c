/**
 * Módulo de comunicação TCP do servidor.
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
#include <errno.h>

#include "network_server.h"
#include "list_skel.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"

static volatile int shutdown_requested = 0;

int network_server_init(short port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    // Permitir reutilização rápida do porto
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 5) < 0)
    {
        perror("listen");
        close(sockfd);
        return -1;
    }

    printf("Servidor a escutar no porto %d\n", port);
    return sockfd;
}

MessageT *network_receive(int client_socket)
{
    if (client_socket < 0)
    {
        return NULL;
    }

    // Receber tamanho da mensagem (4 bytes)
    uint32_t net_size;
    if (read_all(client_socket, &net_size, sizeof(net_size)) < 0)
    {
        return NULL;
    }

    uint32_t msg_size = ntohl(net_size);
    if (msg_size == 0 || msg_size > 10 * 1024 * 1024) // Limite de 10MB
    {
        return NULL;
    }

    // Receber mensagem serializada
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL)
    {
        return NULL;
    }

    if (read_all(client_socket, buffer, msg_size) < 0)
    {
        free(buffer);
        return NULL;
    }

    // Desserializar
    MessageT *msg = message_t__unpack(NULL, msg_size, buffer);
    free(buffer);

    return msg;
}

int network_send(int client_socket, MessageT *msg)
{
    if (client_socket < 0 || msg == NULL)
    {
        return -1;
    }

    // Serializar mensagem
    size_t msg_size = message_t__get_packed_size(msg);
    uint8_t *buffer = malloc(msg_size);
    if (buffer == NULL)
    {
        return -1;
    }

    message_t__pack(msg, buffer);

    // Enviar tamanho
    uint32_t net_size = htonl((uint32_t)msg_size);
    if (write_all(client_socket, &net_size, sizeof(net_size)) < 0)
    {
        free(buffer);
        return -1;
    }

    // Enviar mensagem
    if (write_all(client_socket, buffer, msg_size) < 0)
    {
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}

int network_main_loop(int listening_socket, struct list_t *list)
{
    if (listening_socket < 0 || list == NULL)
    {
        return -1;
    }

    while (!shutdown_requested)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        printf("A aguardar conexões...\n");
        int client_socket = accept(listening_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0)
        {
            if (errno == EINTR || shutdown_requested)
            {
                break;
            }
            perror("accept");
            continue;
        }

        printf("Cliente conectado: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Loop de atendimento do cliente
        int client_active = 1;
        while (client_active && !shutdown_requested)
        {
            // Receber pedido
            MessageT *request = network_receive(client_socket);
            if (request == NULL)
            {
                printf("Erro ao receber mensagem ou cliente desconectou.\n");
                client_active = 0;
                break;
            }

            // Processar pedido no skeleton
            if (invoke(request, list) < 0)
            {
                printf("Erro ao processar pedido.\n");
                // Criar resposta de erro
                request->opcode = MESSAGE_T__OPCODE__OP_ERROR;
                request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            }

            // Enviar resposta
            if (network_send(client_socket, request) < 0)
            {
                printf("Erro ao enviar resposta.\n");
                client_active = 0;
            }

            message_t__free_unpacked(request, NULL);
        }

        close(client_socket);
        printf("Cliente desconectado.\n");
    }

    return 0;
}

int network_server_close(int socket)
{
    if (socket < 0)
    {
        return -1;
    }

    close(socket);
    return 0;
}

void network_server_request_shutdown(void)
{
    shutdown_requested = 1;
}
