/**
 * Cliente interativo para comunicar com o servidor de lista de carros.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "data.h"

static void print_data(struct data_t *car)
{
    if (car == NULL)
    {
        printf("(dados inválidos)\n");
        return;
    }

    const char *marca_str[] = {"Toyota", "BMW", "Renault", "Audi", "Mercedes"};
    const char *combustivel_str[] = {"Gasolina", "Gasóleo", "Elétrico", "Híbrido"};

    printf("  Modelo: %s\n", car->modelo);
    printf("  Ano: %d\n", car->ano);
    printf("  Preço: %.2f€\n", car->preco);
    printf("  Marca: %s\n", marca_str[car->marca]);
    printf("  Combustível: %s\n", combustivel_str[car->combustivel]);
}

static void print_help(void)
{
    printf("\nComandos disponíveis:\n");
    printf("  add <ano> <preco> <marca> <modelo> <combustivel>\n");
    printf("      Adiciona um carro à lista\n");
    printf("      marca: 0=Toyota, 1=BMW, 2=Renault, 3=Audi, 4=Mercedes\n");
    printf("      combustivel: 0=Gasolina, 1=Gasóleo, 2=Elétrico, 3=Híbrido\n");
    printf("  remove <modelo>\n");
    printf("      Remove o carro com o modelo indicado\n");
    printf("  get_by_marca <marca>\n");
    printf("      Obtém o primeiro carro da marca indicada\n");
    printf("  get_by_year <ano>\n");
    printf("      Obtém todos os carros do ano indicado\n");
    printf("  get_model_list\n");
    printf("      Lista todos os modelos de carros\n");
    printf("  get_list_ordered_by_year\n");
    printf("      Lista todos os carros ordenados por ano\n");
    printf("  size\n");
    printf("      Mostra o número de carros na lista\n");
    printf("  help\n");
    printf("      Mostra esta ajuda\n");
    printf("  quit\n");
    printf("      Termina o cliente\n\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <servidor>:<porto>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s localhost:8080\n", argv[0]);
        return 1;
    }

    // Conectar ao servidor
    struct rlist_t *rlist = rlist_connect(argv[1]);
    if (rlist == NULL)
    {
        fprintf(stderr, "Erro ao conectar ao servidor %s\n", argv[1]);
        return 1;
    }

    printf("Conectado ao servidor %s\n", argv[1]);
    printf("Digite 'help' para ver os comandos disponíveis.\n");

    char line[1024];
    while (1)
    {
        printf("\n> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break;
        }

        // Remover newline
        line[strcspn(line, "\n")] = 0;

        // Parse do comando
        char cmd[256];
        if (sscanf(line, "%255s", cmd) != 1)
        {
            continue;
        }

        if (strcmp(cmd, "quit") == 0)
        {
            break;
        }
        else if (strcmp(cmd, "help") == 0)
        {
            print_help();
        }
        else if (strcmp(cmd, "add") == 0)
        {
            int ano, marca, combustivel;
            float preco;
            char modelo[256];

            if (sscanf(line, "add %d %f %d %255s %d", &ano, &preco, &marca, modelo, &combustivel) != 5)
            {
                printf("Erro: formato inválido. Use: add <ano> <preco> <marca> <modelo> <combustivel>\n");
                continue;
            }

            if (marca < 0 || marca > 4 || combustivel < 0 || combustivel > 3)
            {
                printf("Erro: marca ou combustível inválido.\n");
                continue;
            }

            struct data_t *car = data_create(ano, preco, (enum marca_t)marca, modelo, (enum combustivel_t)combustivel);
            if (car == NULL)
            {
                printf("Erro ao criar dados do carro.\n");
                continue;
            }

            if (rlist_add(rlist, car) == 0)
            {
                printf("Carro adicionado com sucesso.\n");
            }
            else
            {
                printf("Erro ao adicionar carro.\n");
            }

            data_destroy(car);
        }
        else if (strcmp(cmd, "remove") == 0)
        {
            char modelo[256];
            if (sscanf(line, "remove %255s", modelo) != 1)
            {
                printf("Erro: formato inválido. Use: remove <modelo>\n");
                continue;
            }

            int result = rlist_remove_by_model(rlist, modelo);
            if (result == 0)
            {
                printf("Carro removido com sucesso.\n");
            }
            else if (result == 1)
            {
                printf("Carro não encontrado.\n");
            }
            else
            {
                printf("Erro ao remover carro.\n");
            }
        }
        else if (strcmp(cmd, "get_by_marca") == 0)
        {
            int marca;
            if (sscanf(line, "get_by_marca %d", &marca) != 1)
            {
                printf("Erro: formato inválido. Use: get_by_marca <marca>\n");
                continue;
            }

            if (marca < 0 || marca > 4)
            {
                printf("Erro: marca inválida.\n");
                continue;
            }

            struct data_t *car = rlist_get_by_marca(rlist, (enum marca_t)marca);
            if (car != NULL)
            {
                printf("Carro encontrado:\n");
                print_data(car);
                data_destroy(car);
            }
            else
            {
                printf("Nenhum carro encontrado com essa marca.\n");
            }
        }
        else if (strcmp(cmd, "get_by_year") == 0)
        {
            int ano;
            if (sscanf(line, "get_by_year %d", &ano) != 1)
            {
                printf("Erro: formato inválido. Use: get_by_year <ano>\n");
                continue;
            }

            struct data_t **cars = rlist_get_by_year(rlist, ano);
            if (cars != NULL)
            {
                int count = 0;
                for (int i = 0; cars[i] != NULL; i++)
                {
                    printf("\nCarro %d:\n", i + 1);
                    print_data(cars[i]);
                    data_destroy(cars[i]);
                    count++;
                }
                free(cars);

                if (count == 0)
                {
                    printf("Nenhum carro encontrado para o ano %d.\n", ano);
                }
                else
                {
                    printf("\nTotal: %d carro(s) encontrado(s).\n", count);
                }
            }
            else
            {
                printf("Erro ao obter carros.\n");
            }
        }
        else if (strcmp(cmd, "get_model_list") == 0)
        {
            char **models = rlist_get_model_list(rlist);
            if (models != NULL)
            {
                int count = 0;
                printf("Modelos na lista:\n");
                for (int i = 0; models[i] != NULL; i++)
                {
                    printf("  %d. %s\n", i + 1, models[i]);
                    count++;
                }

                if (count == 0)
                {
                    printf("  (lista vazia)\n");
                }

                rlist_free_model_list(models);
            }
            else
            {
                printf("Erro ao obter lista de modelos.\n");
            }
        }
        else if (strcmp(cmd, "get_list_ordered_by_year") == 0)
        {
            // Nota: este comando ordena a lista no servidor e retorna todos os carros
            // Para simplificar, vamos usar get_by_year com valor impossível para trigger
            // Mas na verdade devemos criar um comando específico...
            // Vamos implementar chamando order + obtendo tamanho + get individuais

            // Estratégia melhor: chamar size e depois iterar
            // Mas a API não tem "get by index"...
            // Vamos precisar de outra abordagem

            printf("Comando não implementado nesta versão do cliente.\n");
            printf("Use o servidor para ordenar e depois get_model_list.\n");
        }
        else if (strcmp(cmd, "size") == 0)
        {
            int size = rlist_size(rlist);
            if (size >= 0)
            {
                printf("A lista contém %d carro(s).\n", size);
            }
            else
            {
                printf("Erro ao obter tamanho da lista.\n");
            }
        }
        else
        {
            printf("Comando desconhecido: %s\n", cmd);
            printf("Digite 'help' para ver os comandos disponíveis.\n");
        }
    }

    printf("\nA desconectar...\n");
    rlist_disconnect(rlist);
    printf("Cliente terminado.\n");

    return 0;
}
