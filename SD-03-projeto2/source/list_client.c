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

/**
 * Converte string de marca para enum marca_t
 */
static enum marca_t parse_marca(const char *str) {
    if (strcmp(str, "TOYOTA") == 0) return MARCA_TOYOTA;
    if (strcmp(str, "BMW") == 0) return MARCA_BMW;
    if (strcmp(str, "RENAULT") == 0) return MARCA_RENAULT;
    if (strcmp(str, "AUDI") == 0) return MARCA_AUDI;
    if (strcmp(str, "MERCEDES") == 0) return MARCA_MERCEDES;
    return MARCA_TOYOTA; // default
}

/**
 * Converte enum marca_t para string
 */
static const char *marca_to_string(enum marca_t marca) {
    switch (marca) {
        case MARCA_TOYOTA: return "TOYOTA";
        case MARCA_BMW: return "BMW";
        case MARCA_RENAULT: return "RENAULT";
        case MARCA_AUDI: return "AUDI";
        case MARCA_MERCEDES: return "MERCEDES";
        default: return "UNKNOWN";
    }
}

/**
 * Converte string de combustível para enum combustivel_t
 */
static enum combustivel_t parse_combustivel(const char *str) {
    if (strcmp(str, "GASOLINA") == 0) return COMBUSTIVEL_GASOLINA;
    if (strcmp(str, "GASOLEO") == 0) return COMBUSTIVEL_GASOLEO;
    if (strcmp(str, "ELETRICO") == 0) return COMBUSTIVEL_ELETRICO;
    if (strcmp(str, "HIBRIDO") == 0) return COMBUSTIVEL_HIBRIDO;
    return COMBUSTIVEL_GASOLINA; // default
}

/**
 * Converte enum combustivel_t para string
 */
static const char *combustivel_to_string(enum combustivel_t combustivel) {
    switch (combustivel) {
        case COMBUSTIVEL_GASOLINA: return "GASOLINA";
        case COMBUSTIVEL_GASOLEO: return "GASOLEO";
        case COMBUSTIVEL_ELETRICO: return "ELETRICO";
        case COMBUSTIVEL_HIBRIDO: return "HIBRIDO";
        default: return "UNKNOWN";
    }
}

/**
 * Imprime os dados de um carro
 */
static void print_car(struct data_t *car) {
    if (car == NULL) {
        printf("Carro não encontrado.\n");
        return;
    }
    printf("Ano: %d, Preço: %.2f, Marca: %s, Modelo: %s, Combustível: %s\n",
           car->ano, car->preco, marca_to_string(car->marca),
           car->modelo, combustivel_to_string(car->combustivel));
}

/**
 * Processa o comando 'add'
 */
static void cmd_add(struct rlist_t *rlist, char *line) {
    int ano;
    float preco;
    char marca_str[50], modelo[100], combustivel_str[50];

    // Parse: add <ano> <preco> <marca> <modelo> <combustivel>
    if (sscanf(line, "add %d %f %s %s %s", &ano, &preco, marca_str, modelo, combustivel_str) != 5) {
        printf("Erro: Uso correto: add <ano> <preco> <marca> <modelo> <combustivel>\n");
        printf("Exemplo: add 2020 25000.50 TOYOTA Corolla GASOLINA\n");
        return;
    }

    enum marca_t marca = parse_marca(marca_str);
    enum combustivel_t combustivel = parse_combustivel(combustivel_str);

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

    // Libertar memória local (servidor já recebeu cópia dos dados)
    data_destroy(car);
}

/**
 * Processa o comando 'remove'
 */
static void cmd_remove(struct rlist_t *rlist, char *line) {
    char modelo[100];

    // Parse: remove <modelo>
    if (sscanf(line, "remove %s", modelo) != 1) {
        printf("Erro: Uso correto: remove <modelo>\n");
        printf("Exemplo: remove Corolla\n");
        return;
    }

    int result = rlist_remove_by_model(rlist, modelo);
    if (result == 0) {
        printf("Carro removido com sucesso.\n");
    } else {
        printf("Carro não encontrado.\n");
    }
}

/**
 * Processa o comando 'get_by_marca'
 */
static void cmd_get_by_marca(struct rlist_t *rlist, char *line) {
    char marca_str[50];

    // Parse: get_by_marca <marca>
    if (sscanf(line, "get_by_marca %s", marca_str) != 1) {
        printf("Erro: Uso correto: get_by_marca <marca>\n");
        printf("Exemplo: get_by_marca TOYOTA\n");
        return;
    }

    enum marca_t marca = parse_marca(marca_str);
    struct data_t *car = rlist_get_by_marca(rlist, marca);

    if (car != NULL) {
        print_car(car);
        data_destroy(car);
    } else {
        printf("Carro não encontrado.\n");
    }
}

/**
 * Processa o comando 'get_by_year'
 */
static void cmd_get_by_year(struct rlist_t *rlist, char *line) {
    int ano;

    // Parse: get_by_year <ano>
    if (sscanf(line, "get_by_year %d", &ano) != 1) {
        printf("Erro: Uso correto: get_by_year <ano>\n");
        printf("Exemplo: get_by_year 2020\n");
        return;
    }

    struct data_t **cars = rlist_get_by_year(rlist, ano);

    if (cars == NULL) {
        printf("Erro ao obter carros.\n");
        return;
    }

    int count = 0;
    while (cars[count] != NULL) {
        print_car(cars[count]);
        data_destroy(cars[count]);
        count++;
    }

    if (count == 0) {
        printf("Nenhum carro encontrado para o ano %d.\n", ano);
    } else {
        printf("Total: %d carro(s) encontrado(s).\n", count);
    }

    free(cars);
}

/**
 * Processa o comando 'get_list_ordered_by_year'
 */
static void cmd_get_list_ordered_by_year(struct rlist_t *rlist) {
    if (rlist_order_by_year(rlist) == 0) {
        printf("\nModelos ordenados por ano:\n");
        printf("---------------------------\n");

        char **models = rlist_get_model_list(rlist);

        if (models == NULL) {
            printf("Erro ao obter lista de modelos.\n");
            return;
        }

        int count = 0;
        while (models[count] != NULL) {
            printf("%d. %s\n", count + 1, models[count]);
            count++;
        }

        if (count == 0) {
            printf("Lista vazia.\n");
        } else {
            printf("---------------------------\n");
            printf("Total: %d modelo(s) ordenado(s).\n", count);
        }

        rlist_free_model_list(models);
    } else {
        printf("Erro ao ordenar lista.\n");
    }
}

/**
 * Processa o comando 'get_model_list'
 */
static void cmd_get_model_list(struct rlist_t *rlist) {
    char **models = rlist_get_model_list(rlist);

    if (models == NULL) {
        printf("Erro ao obter lista de modelos.\n");
        return;
    }

    int count = 0;
    while (models[count] != NULL) {
        printf("%d. %s\n", count + 1, models[count]);
        count++;
    }

    if (count == 0) {
        printf("Lista vazia.\n");
    } else {
        printf("Total: %d modelo(s).\n", count);
    }

    rlist_free_model_list(models);
}

/**
 * Processa o comando 'size'
 */
static void cmd_size(struct rlist_t *rlist) {
    int size = rlist_size(rlist);

    if (size >= 0) {
        printf("Número de carros na lista: %d\n", size);
    } else {
        printf("Erro ao obter tamanho da lista.\n");
    }
}

/**
 * Mostra menu de ajuda
 */
static void print_help() {
    printf("\n=== COMANDOS DISPONÍVEIS ===\n");
    printf("  add <ano> <preco> <marca> <modelo> <combustivel>\n");
    printf("      - Adiciona um carro à lista\n");
    printf("      - Exemplo: add 2020 25000.50 TOYOTA Corolla GASOLINA\n\n");
    printf("  remove <modelo>\n");
    printf("      - Remove um carro pelo modelo\n");
    printf("      - Exemplo: remove Corolla\n\n");
    printf("  get_by_marca <marca>\n");
    printf("      - Obtém o primeiro carro da marca especificada\n");
    printf("      - Exemplo: get_by_marca TOYOTA\n\n");
    printf("  get_by_year <ano>\n");
    printf("      - Obtém todos os carros de um ano específico\n");
    printf("      - Exemplo: get_by_year 2020\n\n");
    printf("  get_list_ordered_by_year\n");
    printf("      - Ordena a lista por ano de fabrico\n\n");
    printf("  get_model_list\n");
    printf("      - Lista todos os modelos de carros\n\n");
    printf("  size\n");
    printf("      - Mostra o número de carros na lista\n\n");
    printf("  help\n");
    printf("      - Mostra esta mensagem de ajuda\n\n");
    printf("  quit\n");
    printf("      - Sai do programa\n");
    printf("============================\n\n");
}

/**
 * Programa principal do cliente
 */
int main(int argc, char **argv) {
    struct rlist_t *rlist;
    char line[MAX_LINE];

    /* ========================================================================
     * 1. VALIDAR ARGUMENTOS
     * ======================================================================== */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <servidor>:<porta>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s localhost:12345\n", argv[0]);
        return -1;
    }

    /* ========================================================================
     * 2. IGNORAR SIGPIPE
     *
     * Evita que o programa termine se o servidor fechar a conexão abruptamente
     * ======================================================================== */
    signal(SIGPIPE, SIG_IGN);

    /* ========================================================================
     * 3. CONECTAR AO SERVIDOR
     * ======================================================================== */
    printf("Conectando ao servidor %s...\n", argv[1]);
    rlist = rlist_connect(argv[1]);

    if (rlist == NULL) {
        fprintf(stderr, "Erro: Não foi possível conectar ao servidor %s\n", argv[1]);
        fprintf(stderr, "Verifique se o servidor está em execução e o endereço está correto.\n");
        return -1;
    }

    printf("✓ Conectado com sucesso!\n");
    printf("Digite 'help' para ver os comandos disponíveis.\n\n");

    /* ========================================================================
     * 4. LOOP PRINCIPAL - PROCESSAR COMANDOS
     * ======================================================================== */
    while (1) {
        printf(">>> ");
        fflush(stdout);

        // Ler linha do utilizador
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remover newline do final
        line[strcspn(line, "\n")] = 0;

        // Ignorar linhas vazias
        if (strlen(line) == 0) {
            continue;
        }

        // Processar comando
        if (strcmp(line, "quit") == 0) {
            printf("A desconectar...\n");
            break;
        }
        else if (strcmp(line, "help") == 0) {
            print_help();
        }
        else if (strncmp(line, "add ", 4) == 0) {
            cmd_add(rlist, line);
        }
        else if (strncmp(line, "remove ", 7) == 0) {
            cmd_remove(rlist, line);
        }
        else if (strncmp(line, "get_by_marca ", 13) == 0) {
            cmd_get_by_marca(rlist, line);
        }
        else if (strncmp(line, "get_by_year ", 12) == 0) {
            cmd_get_by_year(rlist, line);
        }
        else if (strcmp(line, "get_list_ordered_by_year") == 0) {
            cmd_get_list_ordered_by_year(rlist);
        }
        else if (strcmp(line, "get_model_list") == 0) {
            cmd_get_model_list(rlist);
        }
        else if (strcmp(line, "size") == 0) {
            cmd_size(rlist);
        }
        else {
            printf("Comando não reconhecido: '%s'\n", line);
            printf("Digite 'help' para ver os comandos disponíveis.\n");
        }
    }

    /* ========================================================================
     * 5. DESCONECTAR E LIMPAR RECURSOS
     * ======================================================================== */
    if (rlist_disconnect(rlist) == 0) {
        printf("✓ Desconectado com sucesso.\n");
    } else {
        printf("Aviso: Erro ao desconectar.\n");
    }

    printf("Cliente terminado.\n");

    return 0;
}
