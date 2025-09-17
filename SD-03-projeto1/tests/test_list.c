/**
 * @file test_list.c
 * @brief Ficheiro de teste para a lista de carros.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 * Autor: José Cecílio
 * Data: 13/09/2025
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"
#include "data.h"

int testCreateList()
{
	struct list_t *list = list_create();
	printf("Teste: Criar lista - ");
	int result = (list != NULL) && (list_size(list) == 0);
	list_destroy(list);

	printf("%s\n", result ? "passou" : "não passou");
	return result;
}

int testAddCars()
{
	struct list_t *list = list_create();
	struct data_t *car1 = data_create(2020, 18000.0, MARCA_RENAULT, "Clio", COMBUSTIVEL_GASOLEO);
	struct data_t *car2 = data_create(2022, 25000.0, MARCA_TOYOTA, "Corolla", COMBUSTIVEL_GASOLINA);
	int result = (list_add(list, car1) == 0) && (list_add(list, car2) == 0) && (list_size(list) == 2);
	list_destroy(list);

	printf("Teste: Adicionar carros - %s\n", result ? "passou" : "não passou");
	return result;
}

int testRemoveByModel()
{
	struct list_t *list = list_create();
	struct data_t *car = data_create(2021, 22000.0, MARCA_BMW, "320d", COMBUSTIVEL_GASOLEO);
	list_add(list, car);
	int result = (list_remove_by_model(list, "320d") == 0) && (list_size(list) == 0);
	list_destroy(list);

	printf("Teste: Remover por modelo - %s\n", result ? "passou" : "não passou");
	return result;
}

int testGetByMarca()
{
	struct list_t *list = list_create();
	struct data_t *car = data_create(2023, 30000.0, MARCA_AUDI, "A3", COMBUSTIVEL_GASOLINA);
	list_add(list, car);
	struct data_t *found = list_get_by_marca(list, MARCA_AUDI);
	int result = (found != NULL) && (strcmp(found->modelo, "A3") == 0);
	list_destroy(list);

	printf("Teste: Obter por marca - %s\n", result ? "passou" : "não passou");
	return result;
}

int testGetByYear()
{
	struct list_t *list = list_create();
	list_add(list, data_create(2020, 15000.0, MARCA_RENAULT, "Twingo", COMBUSTIVEL_GASOLINA));
	list_add(list, data_create(2020, 16000.0, MARCA_RENAULT, "Captur", COMBUSTIVEL_GASOLEO));
	struct data_t **cars = list_get_by_year(list, 2020);
	int count = 0;
	while (cars && cars[count])
		count++;
	int result = (count == 2);
	free(cars);
	list_destroy(list);

	printf("Teste: Obter por ano - %s\n", result ? "passou" : "não passou");
	return result;
}

int testOrderByYear()
{
	struct list_t *list = list_create();
	list_add(list, data_create(2022, 20000.0, MARCA_TOYOTA, "Yaris", COMBUSTIVEL_HIBRIDO));
	list_add(list, data_create(2020, 18000.0, MARCA_BMW, "X1", COMBUSTIVEL_GASOLEO));
	list_add(list, data_create(2021, 19000.0, MARCA_AUDI, "Q2", COMBUSTIVEL_GASOLINA));
	list_order_by_year(list);
	struct data_t **cars = list_get_by_year(list, 2020);
	int result = (cars && strcmp(cars[0]->modelo, "X1") == 0);
	free(cars);
	list_destroy(list);

	printf("Teste: Ordenar por ano - %s\n", result ? "passou" : "não passou");
	return result;
}

int testGetModelList()
{
	struct list_t *list = list_create();
	list_add(list, data_create(2021, 21000.0, MARCA_MERCEDES, "Classe A", COMBUSTIVEL_GASOLEO));
	list_add(list, data_create(2022, 22000.0, MARCA_MERCEDES, "Classe B", COMBUSTIVEL_GASOLINA));
	char **models = list_get_model_list(list);
	int result = (models != NULL) && (strcmp(models[0], "Classe A") == 0) && (strcmp(models[1], "Classe B") == 0);
	list_free_model_list(models);
	list_destroy(list);

	printf("Teste: Obter lista de modelos - %s\n", result ? "passou" : "não passou");
	return result;
}

int main()
{
	int score = 0;

	score += testCreateList();
	score += testAddCars();
	score += testRemoveByModel();
	score += testGetByMarca();
	score += testGetByYear();
	score += testOrderByYear();
	score += testGetModelList();

	printf("\nResultado dos testes: %d/7\n", score);
	return (score == 7) ? 0 : -1;
}
