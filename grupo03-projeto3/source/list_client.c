/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "client_stub.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE 1024

static enum marca_t parse_marca(int marca_num) {
    switch (marca_num) {
        case 0: return MARCA_TOYOTA;
        case 1: return MARCA_BMW;
        case 2: return MARCA_RENAULT;
        case 3: return MARCA_AUDI;
        case 4: return MARCA_MERCEDES;
        default: return MARCA_TOYOTA;
    }
}

static enum combustivel_t parse_combustivel(int combustivel_num) {
    switch (combustivel_num) {
        case 0: return COMBUSTIVEL_GASOLINA;
        case 1: return COMBUSTIVEL_GASOLEO;
        case 2: return COMBUSTIVEL_ELETRICO;
        case 3: return COMBUSTIVEL_HIBRIDO;
        default: return COMBUSTIVEL_GASOLINA;
    }
}

static void print_car(struct data_t *car) {
    if (car == NULL) {
        printf("Carro não encontrado.\n");
        return;
    }
    printf("Modelo: %s\n", car->modelo);
    printf("Ano: %d\n", car->ano);
    printf("Preço: %.2f\n", car->preco);
    printf("Marca: %d\n", car->marca);
    printf("Combustível: %d\n", car->combustivel);
}

static void cmd_add(struct rlist_t *rlist, char *line) {
    int ano;
    float preco;
    int marca_num, combustivel_num;
    char modelo[100];

    if (sscanf(line, "add %s %d %f %d %d", modelo, &ano, &preco, &marca_num, &combustivel_num) != 5) {
        printf("Uso: add <modelo> <ano> <preco> <marca:0-4> <combustivel:0-3>\n");
        return;
    }

    enum marca_t marca = parse_marca(marca_num);
    enum combustivel_t combustivel = parse_combustivel(combustivel_num);

    struct data_t *car = data_create(ano, preco, marca, modelo, combustivel);
    if (car == NULL) {
        printf("Erro ao criar carro.\n");
        return;
    }

    if (rlist_add(rlist, car) == 0) {
        printf("Carro adicionado com sucesso.\n");
    } else {
        printf("Erro ao adicionar carro.\n");
    }

    data_destroy(car);
}

static void cmd_remove(struct rlist_t *rlist, char *line) {
    char modelo[100];

    if (sscanf(line, "remove %s", modelo) != 1) {
        printf("Uso: remove <modelo>\n");
        return;
    }

    int result = rlist_remove_by_model(rlist, modelo);
    if (result == 0) {
        printf("Carro removido.\n");
    } else {
        printf("Carro não encontrado.\n");
    }
}

static void cmd_get_by_marca(struct rlist_t *rlist, char *line) {
    int marca_num;

    if (sscanf(line, "get_by_marca %d", &marca_num) != 1) {
        printf("Uso: get_by_marca <marca:0-4>\n");
        return;
    }

    enum marca_t marca = parse_marca(marca_num);
    struct data_t *car = rlist_get_by_marca(rlist, marca);

    if (car != NULL) {
        print_car(car);
        data_destroy(car);
    } else {
        printf("Carro não encontrado.\n");
    }
}

static void cmd_get_by_year(struct rlist_t *rlist, char *line) {
    int ano;

    if (sscanf(line, "get_by_year %d", &ano) != 1) {
        printf("Uso: get_by_year <ano>\n");
        return;
    }

    struct data_t **cars = rlist_get_by_year(rlist, ano);

    if (cars == NULL) {
        printf("Nenhum carro encontrado para o ano especificado.\n");
        return;
    }

    int count = 0;
    while (cars[count] != NULL) {
        print_car(cars[count]);
        data_destroy(cars[count]);
        count++;
    }

    if (count == 0) {
        printf("Nenhum carro encontrado para o ano especificado.\n");
    }

    free(cars);
}

static void cmd_get_list_ordered_by_year(struct rlist_t *rlist) {
    if (rlist_order_by_year(rlist) != 0) {
        printf("Erro ao ordenar a lista por cada ano.\n");
        return;
    }

    struct data_t **cars = rlist_get_by_year(rlist, -1);
    if (cars == NULL) {
        printf("Erro ao obter os carros por cada ano.\n");
        return;
    }

    int i = 0;
    while (cars[i] != NULL) {
        print_car(cars[i]);
        data_destroy(cars[i]);
        i++;
    }
    free(cars);
}

static void cmd_get_model_list(struct rlist_t *rlist) {
    char **models = rlist_get_model_list(rlist);

    if (models == NULL) {
        printf("Erro ao obter lista de modelos.\n");
        return;
    }

    int count = 0;
    while (models[count] != NULL) {
        printf("Modelo: %s\n", models[count]);
        count++;
    }

    rlist_free_model_list(models);
}

static void cmd_size(struct rlist_t *rlist) {
    int size = rlist_size(rlist);

    if (size >= 0) {
        printf("List size: %d\n", size);
    } else {
        printf("Erro ao obter tamanho da lista.\n");
    }
}

static void print_help() {
    printf("Comandos disponíveis:\n");
    printf("  add <modelo> <ano> <preco> <marca:0-4> <combustivel:0-3>\n");
    printf("  remove <modelo>\n");
    printf("  get_by_marca <marca:0-4>\n");
    printf("  get_by_year <ano>\n");
    printf("  get_list_ordered_by_year\n");
    printf("  size\n");
    printf("  get_model_list\n");
    printf("  help\n");
    printf("  quit\n");
}

int main(int argc, char **argv) {
    struct rlist_t *rlist;
    char line[MAX_LINE];

    if (argc != 2) {
        fprintf(stderr, "Utilização: %s <servidor>:<porta>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s localhost:12345\n", argv[0]);
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    rlist = rlist_connect(argv[1]);

    if (rlist == NULL) {
        return -1;
    }

    char address_copy[256];
    strncpy(address_copy, argv[1], sizeof(address_copy) - 1);
    address_copy[sizeof(address_copy) - 1] = '\0';

    char *colon = strchr(address_copy, ':');
    if (colon != NULL) {
        *colon = '\0';
    }

    printf("Ligado a %s\n", address_copy);
    print_help();

    while (1) {
        printf("Command: ");

        if (fgets(line, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\r\n")] = 0;

        if (strlen(line) == 0) continue;
        if (strcmp(line, "quit") == 0) break;
        if (strcmp(line, "help") == 0) print_help();
        else if (strncmp(line, "add ", 4) == 0) cmd_add(rlist, line);
        else if (strncmp(line, "remove ", 7) == 0) cmd_remove(rlist, line);
        else if (strncmp(line, "get_by_marca ", 13) == 0) cmd_get_by_marca(rlist, line);
        else if (strncmp(line, "get_by_year ", 12) == 0) cmd_get_by_year(rlist, line);
        else if (strcmp(line, "get_list_ordered_by_year") == 0) cmd_get_list_ordered_by_year(rlist);
        else if (strcmp(line, "get_model_list") == 0) cmd_get_model_list(rlist);
        else if (strcmp(line, "size") == 0) cmd_size(rlist);
        else printf("Comando inválido. Escreve 'help' para ajuda.\n");
    }

    rlist_disconnect(rlist);
    return 0;
}
