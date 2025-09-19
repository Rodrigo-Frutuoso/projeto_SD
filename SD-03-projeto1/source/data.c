/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

 #include "../include/data.h"
 #include <stdlib.h>
 #include <string.h>

 struct data_t *data_create(int ano, float preco, enum marca_t marca, const char *modelo, enum combustivel_t combustivel)
{
    struct data_t *data = malloc(sizeof(struct data_t));
    if (data == NULL) return NULL;

    data->ano = ano;
    data->preco = preco;
    data->marca = marca;
    data->modelo = strdup(modelo);
    data->combustivel = combustivel;

    return data;
}

int data_destroy(struct data_t *data)
{
    if (data == NULL) return -1;

    free(data->modelo);
    free(data);
    return 0;
}

struct data_t *data_dup(struct data_t *data)
{
    if (data == NULL) return NULL;
    return data_create(data->ano, data->preco, data->marca, data->modelo, data->combustivel);
}

int data_replace(struct data_t *data, int ano, float preco, enum marca_t marca, const char *modelo, enum combustivel_t
combustivel)
{
    if (data == NULL) return -1;

    data->ano = ano;
    data->preco = preco;
    data->marca = marca;
    free(data->modelo);
    data->modelo = strdup(modelo);
    data->combustivel = combustivel;

    return 0;
}
