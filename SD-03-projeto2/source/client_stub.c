/**
 * Client stub - camada de abstração RPC do cliente.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "data.h"
#include "sdmessage.pb-c.h"

// Função auxiliar para converter data_t para Data (protobuf)
static Data *data_to_protobuf(struct data_t *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    Data *pb_data = malloc(sizeof(Data));
    if (pb_data == NULL)
    {
        return NULL;
    }

    data__init(pb_data);
    pb_data->ano = data->ano;
    pb_data->preco = data->preco;
    pb_data->marca = (Marca)data->marca;
    pb_data->modelo = strdup(data->modelo != NULL ? data->modelo : "");
    pb_data->combustivel = (Combustivel)data->combustivel;

    if (pb_data->modelo == NULL)
    {
        free(pb_data);
        return NULL;
    }

    return pb_data;
}

// Função auxiliar para converter Data (protobuf) para data_t
static struct data_t *protobuf_to_data(Data *pb_data)
{
    if (pb_data == NULL)
    {
        return NULL;
    }

    return data_create(pb_data->ano, pb_data->preco,
                       (enum marca_t)pb_data->marca,
                       pb_data->modelo,
                       (enum combustivel_t)pb_data->combustivel);
}

struct rlist_t *rlist_connect(char *address_port)
{
    if (address_port == NULL)
    {
        return NULL;
    }

    // Fazer parse de "hostname:port"
    char *colon = strchr(address_port, ':');
    if (colon == NULL)
    {
        return NULL;
    }

    size_t addr_len = colon - address_port;
    char *address = malloc(addr_len + 1);
    if (address == NULL)
    {
        return NULL;
    }

    strncpy(address, address_port, addr_len);
    address[addr_len] = '\0';

    int port = atoi(colon + 1);
    if (port <= 0 || port > 65535)
    {
        free(address);
        return NULL;
    }

    // Criar estrutura rlist_t
    struct rlist_t *rlist = malloc(sizeof(struct rlist_t));
    if (rlist == NULL)
    {
        free(address);
        return NULL;
    }

    rlist->server_address = address;
    rlist->server_port = port;
    rlist->sockfd = -1;

    // Estabelecer conexão
    if (network_connect(rlist) < 0)
    {
        free(address);
        free(rlist);
        return NULL;
    }

    return rlist;
}

int rlist_disconnect(struct rlist_t *rlist)
{
    if (rlist == NULL)
    {
        return -1;
    }

    network_close(rlist);
    free(rlist->server_address);
    free(rlist);
    return 0;
}

int rlist_add(struct rlist_t *rlist, struct data_t *car)
{
    if (rlist == NULL || car == NULL)
    {
        return -1;
    }

    // Criar mensagem protobuf
    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_ADD;
    msg.c_type = MESSAGE_T__C_TYPE__CT_DATA;

    Data *pb_data = data_to_protobuf(car);
    if (pb_data == NULL)
    {
        return -1;
    }

    msg.data = pb_data;

    // Enviar e receber resposta
    MessageT *response = network_send_receive(rlist, &msg);

    free(pb_data->modelo);
    free(pb_data);

    if (response == NULL)
    {
        return -1;
    }

    int result = -1;
    if (response->opcode == MESSAGE_T__OPCODE__OP_ADD + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
    {
        result = (response->result == 0) ? 0 : -1;
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_remove_by_model(struct rlist_t *rlist, const char *modelo)
{
    if (rlist == NULL || modelo == NULL)
    {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_MODEL;

    Data pb_data = DATA__INIT;
    pb_data.modelo = (char *)modelo;
    msg.data = &pb_data;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return -1;
    }

    int result = -1;
    if (response->opcode == MESSAGE_T__OPCODE__OP_DEL + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
    {
        result = response->result;
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

struct data_t *rlist_get_by_marca(struct rlist_t *rlist, enum marca_t marca)
{
    if (rlist == NULL)
    {
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_MARCA;

    Data pb_data = DATA__INIT;
    pb_data.marca = (Marca)marca;
    msg.data = &pb_data;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return NULL;
    }

    struct data_t *result = NULL;
    if (response->opcode == MESSAGE_T__OPCODE__OP_GET + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_DATA &&
        response->data != NULL)
    {
        result = protobuf_to_data(response->data);
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

struct data_t **rlist_get_by_year(struct rlist_t *rlist, int ano)
{
    if (rlist == NULL)
    {
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_YEAR;

    Data pb_data = DATA__INIT;
    pb_data.ano = ano;
    msg.data = &pb_data;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return NULL;
    }

    struct data_t **result = NULL;
    if (response->opcode == MESSAGE_T__OPCODE__OP_GET + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_LIST)
    {
        size_t count = response->n_cars;
        result = malloc(sizeof(struct data_t *) * (count + 1));
        if (result != NULL)
        {
            for (size_t i = 0; i < count; i++)
            {
                result[i] = protobuf_to_data(response->cars[i]);
                if (result[i] == NULL)
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        data_destroy(result[j]);
                    }
                    free(result);
                    result = NULL;
                    break;
                }
            }
            if (result != NULL)
            {
                result[count] = NULL;
            }
        }
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_order_by_year(struct rlist_t *rlist)
{
    if (rlist == NULL)
    {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_ORDER;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return -1;
    }

    int result = -1;
    if (response->opcode == MESSAGE_T__OPCODE__OP_ORDER + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
    {
        result = (response->result == 0) ? 0 : -1;
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_size(struct rlist_t *rlist)
{
    if (rlist == NULL)
    {
        return -1;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return -1;
    }

    int result = -1;
    if (response->opcode == MESSAGE_T__OPCODE__OP_SIZE + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_RESULT)
    {
        result = response->result;
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

char **rlist_get_model_list(struct rlist_t *rlist)
{
    if (rlist == NULL)
    {
        return NULL;
    }

    MessageT msg = MESSAGE_T__INIT;
    msg.opcode = MESSAGE_T__OPCODE__OP_GETMODELS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rlist, &msg);
    if (response == NULL)
    {
        return NULL;
    }

    char **result = NULL;
    if (response->opcode == MESSAGE_T__OPCODE__OP_GETMODELS + 1 &&
        response->c_type == MESSAGE_T__C_TYPE__CT_LIST)
    {
        size_t count = response->n_models;
        result = malloc(sizeof(char *) * (count + 1));
        if (result != NULL)
        {
            for (size_t i = 0; i < count; i++)
            {
                result[i] = strdup(response->models[i]);
                if (result[i] == NULL)
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        free(result[j]);
                    }
                    free(result);
                    result = NULL;
                    break;
                }
            }
            if (result != NULL)
            {
                result[count] = NULL;
            }
        }
    }

    message_t__free_unpacked(response, NULL);
    return result;
}

int rlist_free_model_list(char **models)
{
    if (models == NULL)
    {
        return -1;
    }

    for (int i = 0; models[i] != NULL; i++)
    {
        free(models[i]);
    }
    free(models);
    return 0;
}
