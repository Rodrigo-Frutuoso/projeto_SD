/**
 * Implementação da estrutura de dados (data_t).
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdlib.h>
#include <string.h>
#include "data.h"

struct data_t *data_create(int ano, float preco, enum marca_t marca, const char *modelo, enum combustivel_t combustivel)
{
    if (modelo == NULL)
    {
        return NULL;
    }

    struct data_t *data = malloc(sizeof(struct data_t));
    if (data == NULL)
    {
        return NULL;
    }

    data->modelo = strdup(modelo);
    if (data->modelo == NULL)
    {
        free(data);
        return NULL;
    }

    data->ano = ano;
    data->preco = preco;
    data->marca = marca;
    data->combustivel = combustivel;

    return data;
}

int data_destroy(struct data_t *data)
{
    if (data == NULL)
    {
        return -1;
    }

    free(data->modelo);
    free(data);
    return 0;
}

struct data_t *data_dup(struct data_t *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    return data_create(data->ano, data->preco, data->marca, data->modelo, data->combustivel);
}

int data_replace(struct data_t *data, int ano, float preco, enum marca_t marca, const char *modelo, enum combustivel_t combustivel)
{
    if (data == NULL || modelo == NULL)
    {
        return -1;
    }

    char *new_modelo = strdup(modelo);
    if (new_modelo == NULL)
    {
        return -1;
    }

    free(data->modelo);
    data->modelo = new_modelo;
    data->ano = ano;
    data->preco = preco;
    data->marca = marca;
    data->combustivel = combustivel;

    return 0;
}
