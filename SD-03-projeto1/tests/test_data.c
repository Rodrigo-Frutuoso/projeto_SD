
/**
 * @file test_data.c
 * @brief Testes para o módulo data.c
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

int testCreateDestroy()
{
    printf("Teste: data_create e data_destroy - ");
    struct data_t *car = data_create(2022, 25000.0, MARCA_TOYOTA, "Corolla", COMBUSTIVEL_GASOLINA);
    int result = (car != NULL) && (car->ano == 2022) && (car->preco == 25000.0) &&
                 (car->marca == MARCA_TOYOTA) && (strcmp(car->modelo, "Corolla") == 0) &&
                 (car->combustivel == COMBUSTIVEL_GASOLINA);
    result = result && (data_destroy(car) == 0);

    printf("%s\n", result ? "passou" : "não passou");
    return result;
}

int testDuplicate()
{
    printf("Teste: data_dup - ");
    struct data_t *original = data_create(2021, 18000.0, MARCA_RENAULT, "Clio", COMBUSTIVEL_GASOLEO);
    struct data_t *copy = data_dup(original);
    int result = (copy != NULL) && (copy != original) &&
                 (copy->ano == original->ano) &&
                 (copy->preco == original->preco) &&
                 (copy->marca == original->marca) &&
                 (strcmp(copy->modelo, original->modelo) == 0) &&
                 (copy->combustivel == original->combustivel);
    data_destroy(original);
    data_destroy(copy);

    printf("%s\n", result ? "passou" : "não passou");
    return result;
}

int testReplace()
{
    printf("Teste: data_replace - ");
    struct data_t *car = data_create(2020, 15000.0, MARCA_BMW, "X1", COMBUSTIVEL_GASOLEO);
    int result = (data_replace(car, 2023, 32000.0, MARCA_AUDI, "Q5", COMBUSTIVEL_HIBRIDO) == 0);
    result = result && (car->ano == 2023) && (car->preco == 32000.0) &&
             (car->marca == MARCA_AUDI) && (strcmp(car->modelo, "Q5") == 0) &&
             (car->combustivel == COMBUSTIVEL_HIBRIDO);
    data_destroy(car);

    printf("%s\n", result ? "passou" : "não passou");
    return result;
}

int main()
{
    int score = 0;

    score += testCreateDestroy();
    score += testDuplicate();
    score += testReplace();

    printf("\nResultado dos testes: %d/3\n", score);
    return (score == 3) ? 0 : -1;
}
