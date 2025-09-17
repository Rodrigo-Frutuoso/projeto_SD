
/**
 * @file test_serialization.c
 * @brief Testes para serialização de carros e listas de carros.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 * Autor: José Cecílio
 * Data: 13/09/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "data.h"
#include "list.h"
#include "serialization.h"

int testSerializeCar()
{
    printf("Teste: Serializar e desserializar carro - ");

    struct data_t *car = data_create(2022, 25000.0, MARCA_TOYOTA, "Corolla", COMBUSTIVEL_GASOLINA);

    char *buffer = NULL;
    int size = car_to_buffer(car, &buffer);
    int result = (size > 0 && buffer != NULL);

    struct data_t *car2 = buffer_to_car(buffer);
    result = result && car2 != NULL;
    result = result && car2->ano == car->ano;
    result = result && car2->preco == car->preco;
    result = result && car2->marca == car->marca;
    result = result && strcmp(car2->modelo, car->modelo) == 0;
    result = result && car2->combustivel == car->combustivel;

    free(buffer);
    data_destroy(car);
    data_destroy(car2);

    printf("%s\n", result ? "passou" : "não passou");
    return result;
}

int testSerializeList()
{
    printf("Teste: Serializar e desserializar lista de carros - ");

    struct list_t *list = list_create();
    list_add(list, data_create(2020, 18000.0, MARCA_RENAULT, "Clio", COMBUSTIVEL_GASOLEO));
    list_add(list, data_create(2021, 22000.0, MARCA_BMW, "320d", COMBUSTIVEL_GASOLEO));
    list_add(list, data_create(2022, 27000.0, MARCA_AUDI, "A4", COMBUSTIVEL_GASOLINA));

    char *buffer = NULL;
    int size = car_list_to_buffer(list, &buffer);
    int result = (size > 0 && buffer != NULL);

    struct list_t *list2 = buffer_to_car_list(buffer);
    result = result && list2 != NULL;
    result = result && list_size(list2) == 3;

    char **models = list_get_model_list(list2);
    result = result && models != NULL;
    result = result && strcmp(models[0], "Clio") == 0;
    result = result && strcmp(models[1], "320d") == 0;
    result = result && strcmp(models[2], "A4") == 0;
    result = result && models[3] == NULL;

    list_free_model_list(models);
    list_destroy(list);
    list_destroy(list2);
    free(buffer);

    printf("%s\n", result ? "passou" : "não passou");
    return result;
}

int main()
{
    int score = 0;
    score += testSerializeCar();
    score += testSerializeList();

    printf("\nResultado dos testes de serialização: %d/2\n", score);
    return (score == 2) ? 0 : -1;
}
