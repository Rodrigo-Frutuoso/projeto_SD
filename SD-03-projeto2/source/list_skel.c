/**
 * Skeleton da lista - processa operações RPC na lista do servidor.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list_skel.h"
#include "list.h"
#include "data.h"
#include "sdmessage.pb-c.h"

// Converter Data (protobuf) para data_t
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

// Converter data_t para Data (protobuf) - aloca memória
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

struct list_t *list_skel_init()
{
    return list_create();
}

int list_skel_destroy(struct list_t *list)
{
    return list_destroy(list);
}

int invoke(MessageT *msg, struct list_t *list)
{
    if (msg == NULL || list == NULL)
    {
        return -1;
    }

    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_ADD:
    {
        // add <data>
        if (msg->c_type != MESSAGE_T__C_TYPE__CT_DATA || msg->data == NULL)
        {
            return -1;
        }

        struct data_t *car = protobuf_to_data(msg->data);
        if (car == NULL)
        {
            return -1;
        }

        int result = list_add(list, car);
        data_destroy(car);

        // Preparar resposta
        msg->opcode = MESSAGE_T__OPCODE__OP_ADD + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = result;

        // NÃO libertar msg->data - será liberado por message_t__free_unpacked()
        msg->data = NULL;

        return 0;
    }

    case MESSAGE_T__OPCODE__OP_DEL:
    {
        // remove <model>
        if (msg->c_type != MESSAGE_T__C_TYPE__CT_MODEL || msg->data == NULL || msg->data->modelo == NULL)
        {
            return -1;
        }

        int result = list_remove_by_model(list, msg->data->modelo);

        // Preparar resposta
        msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = result;

        // NÃO libertar msg->data - será liberado por message_t__free_unpacked()
        msg->data = NULL;

        return 0;
    }    case MESSAGE_T__OPCODE__OP_GET:
    {
        if (msg->c_type == MESSAGE_T__C_TYPE__CT_MARCA)
        {
            // get_by_marca <marca>
            if (msg->data == NULL)
            {
                return -1;
            }

            enum marca_t marca = (enum marca_t)msg->data->marca;
            struct data_t *result_data = list_get_by_marca(list, marca);

            // Preparar resposta
            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;

            // NÃO libertar msg->data antiga - será liberado por message_t__free_unpacked()
            if (result_data != NULL)
            {
                msg->c_type = MESSAGE_T__C_TYPE__CT_DATA;
                msg->data = data_to_protobuf(result_data);
                data_destroy(result_data);

                if (msg->data == NULL)
                {
                    return -1;
                }
            }
            else
            {
                msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->data = NULL;
            }

            return 0;
        }
        else if (msg->c_type == MESSAGE_T__C_TYPE__CT_YEAR)
        {
            // get_by_year <year>
            if (msg->data == NULL)
            {
                return -1;
            }

            int ano = msg->data->ano;
            struct data_t **cars = list_get_by_year(list, ano);

            // Preparar resposta
            msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;

            // NÃO libertar msg->data - será liberado por message_t__free_unpacked()
            msg->data = NULL;

            if (cars != NULL)
            {
                // Contar carros
                size_t count = 0;
                while (cars[count] != NULL)
                {
                    count++;
                }

                msg->n_cars = count;
                msg->cars = malloc(sizeof(Data *) * count);
                if (msg->cars == NULL)
                {
                    // Libertar cars
                    for (size_t i = 0; i < count; i++)
                    {
                        data_destroy(cars[i]);
                    }
                    free(cars);
                    return -1;
                }

                for (size_t i = 0; i < count; i++)
                {
                    msg->cars[i] = data_to_protobuf(cars[i]);
                    if (msg->cars[i] == NULL)
                    {
                        // Cleanup
                        for (size_t j = 0; j < i; j++)
                        {
                            free(msg->cars[j]->modelo);
                            free(msg->cars[j]);
                        }
                        free(msg->cars);
                        for (size_t j = 0; j < count; j++)
                        {
                            data_destroy(cars[j]);
                        }
                        free(cars);
                        return -1;
                    }
                    data_destroy(cars[i]);
                }
                free(cars);
            }
            else
            {
                msg->n_cars = 0;
                msg->cars = NULL;
            }

            return 0;
        }
        else
        {
            return -1;
        }
    }

    case MESSAGE_T__OPCODE__OP_SIZE:
    {
        // size
        int size = list_size(list);

        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = size;

        return 0;
    }

    case MESSAGE_T__OPCODE__OP_GETMODELS:
    {
        // get_model_list
        char **models = list_get_model_list(list);

        msg->opcode = MESSAGE_T__OPCODE__OP_GETMODELS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;

        if (models != NULL)
        {
            // Contar modelos
            size_t count = 0;
            while (models[count] != NULL)
            {
                count++;
            }

            msg->n_models = count;
            msg->models = models; // Reutilizar diretamente
        }
        else
        {
            msg->n_models = 0;
            msg->models = NULL;
        }

        return 0;
    }

    case MESSAGE_T__OPCODE__OP_GETLISTBYTEAR:
    {
        // get_list_ordered_by_year
        // Primeiro ordenar
        int order_result = list_order_by_year(list);
        if (order_result < 0)
        {
            return -1;
        }

        // Depois obter todos os carros
        struct data_t **all_cars = list_get_all(list);

        msg->opcode = MESSAGE_T__OPCODE__OP_GETLISTBYTEAR + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_LIST;

        if (all_cars != NULL)
        {
            // Contar carros
            size_t count = 0;
            while (all_cars[count] != NULL)
            {
                count++;
            }

            msg->n_cars = count;
            msg->cars = malloc(sizeof(Data *) * count);
            if (msg->cars == NULL)
            {
                free(all_cars);
                return -1;
            }

            for (size_t i = 0; i < count; i++)
            {
                msg->cars[i] = data_to_protobuf(all_cars[i]);
                if (msg->cars[i] == NULL)
                {
                    // Cleanup
                    for (size_t j = 0; j < i; j++)
                    {
                        free(msg->cars[j]->modelo);
                        free(msg->cars[j]);
                    }
                    free(msg->cars);
                    free(all_cars);
                    return -1;
                }
            }
            free(all_cars); // Não libertar data_t*, apenas o array
        }
        else
        {
            msg->n_cars = 0;
            msg->cars = NULL;
        }

        return 0;
    }

    case MESSAGE_T__OPCODE__OP_ORDER:
    {
        // order_by_year
        int result = list_order_by_year(list);

        msg->opcode = MESSAGE_T__OPCODE__OP_ORDER + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->result = result;

        return 0;
    }

    default:
        return -1;
    }
}
