/**
 * Implementação da lista encadeada de carros.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdlib.h>
#include <string.h>
#include "list.h"

struct list_t *list_create()
{
    struct list_t *list = malloc(sizeof(struct list_t));
    if (list == NULL)
    {
        return NULL;
    }

    list->size = 0;
    list->head = NULL;

    return list;
}

int list_destroy(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }

    struct car_t *current = list->head;
    while (current != NULL)
    {
        struct car_t *next = current->next;
        data_destroy(current->data);
        free(current);
        current = next;
    }

    free(list);
    return 0;
}

int list_add(struct list_t *list, struct data_t *car)
{
    if (list == NULL || car == NULL)
    {
        return -1;
    }

    // Duplicar os dados do carro
    struct data_t *new_data = data_dup(car);
    if (new_data == NULL)
    {
        return -1;
    }

    // Criar novo nó
    struct car_t *new_node = malloc(sizeof(struct car_t));
    if (new_node == NULL)
    {
        data_destroy(new_data);
        return -1;
    }

    new_node->data = new_data;
    new_node->next = NULL;

    // Adicionar no final da lista
    if (list->head == NULL)
    {
        list->head = new_node;
    }
    else
    {
        struct car_t *current = list->head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }

    list->size++;
    return 0;
}

int list_remove_by_model(struct list_t *list, const char *modelo)
{
    if (list == NULL || modelo == NULL)
    {
        return -1;
    }

    struct car_t *current = list->head;
    struct car_t *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->data->modelo, modelo) == 0)
        {
            // Encontrou o carro
            if (prev == NULL)
            {
                // É o primeiro nó
                list->head = current->next;
            }
            else
            {
                prev->next = current->next;
            }

            data_destroy(current->data);
            free(current);
            list->size--;
            return 0; // Sucesso
        }

        prev = current;
        current = current->next;
    }

    return 1; // Não encontrou
}

struct data_t *list_get_by_marca(struct list_t *list, enum marca_t marca)
{
    if (list == NULL)
    {
        return NULL;
    }

    struct car_t *current = list->head;
    while (current != NULL)
    {
        if (current->data->marca == marca)
        {
            return data_dup(current->data);
        }
        current = current->next;
    }

    return NULL; // Não encontrou
}

struct data_t **list_get_by_year(struct list_t *list, int ano)
{
    if (list == NULL)
    {
        return NULL;
    }

    // Primeiro contar quantos carros têm esse ano
    int count = 0;
    struct car_t *current = list->head;
    while (current != NULL)
    {
        if (current->data->ano == ano)
        {
            count++;
        }
        current = current->next;
    }

    // Alocar array (count + 1 para o NULL final)
    struct data_t **result = malloc(sizeof(struct data_t *) * (count + 1));
    if (result == NULL)
    {
        return NULL;
    }

    // Preencher o array
    int index = 0;
    current = list->head;
    while (current != NULL)
    {
        if (current->data->ano == ano)
        {
            result[index] = data_dup(current->data);
            if (result[index] == NULL)
            {
                // Erro ao duplicar, limpar tudo
                for (int i = 0; i < index; i++)
                {
                    data_destroy(result[i]);
                }
                free(result);
                return NULL;
            }
            index++;
        }
        current = current->next;
    }

    result[index] = NULL;
    return result;
}

int list_order_by_year(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }

    if (list->size <= 1)
    {
        return 0; // Já está ordenada
    }

    // Bubble sort na lista encadeada
    int swapped;
    do
    {
        swapped = 0;
        struct car_t *current = list->head;
        struct car_t *prev = NULL;
        struct car_t *next = current->next;

        while (next != NULL)
        {
            if (current->data->ano > next->data->ano)
            {
                // Trocar os nós
                swapped = 1;

                if (prev != NULL)
                {
                    prev->next = next;
                }
                else
                {
                    list->head = next;
                }

                current->next = next->next;
                next->next = current;

                prev = next;
                next = current->next;
            }
            else
            {
                prev = current;
                current = next;
                next = next->next;
            }
        }
    } while (swapped);

    return 0;
}

int list_size(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }

    return list->size;
}

char **list_get_model_list(struct list_t *list)
{
    if (list == NULL)
    {
        return NULL;
    }

    // Alocar array
    char **result = malloc(sizeof(char *) * (list->size + 1));
    if (result == NULL)
    {
        return NULL;
    }

    int index = 0;
    struct car_t *current = list->head;
    while (current != NULL)
    {
        result[index] = strdup(current->data->modelo);
        if (result[index] == NULL)
        {
            // Erro, limpar tudo
            for (int i = 0; i < index; i++)
            {
                free(result[i]);
            }
            free(result);
            return NULL;
        }
        index++;
        current = current->next;
    }

    result[index] = NULL;
    return result;
}

int list_free_model_list(char **models)
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

struct data_t **list_get_all(struct list_t *list)
{
    if (list == NULL)
    {
        return NULL;
    }

    // Alocar array (size + 1 para NULL)
    struct data_t **result = malloc(sizeof(struct data_t *) * (list->size + 1));
    if (result == NULL)
    {
        return NULL;
    }

    int index = 0;
    struct car_t *current = list->head;
    while (current != NULL)
    {
        result[index] = current->data; // Ponteiro direto, não duplica
        index++;
        current = current->next;
    }

    result[index] = NULL;
    return result;
}
