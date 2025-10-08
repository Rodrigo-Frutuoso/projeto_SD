/**
 * Servidor de lista de carros.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "network_server.h"
#include "list_skel.h"

static int server_socket = -1;
static struct list_t *car_list = NULL;

// Handler para SIGINT (Ctrl+C)
static void sigint_handler(int sig)
{
    (void)sig; // Suprimir warning de parâmetro não usado
    printf("\n\nSinal de interrupção recebido. A terminar servidor...\n");

    network_server_request_shutdown();

    if (server_socket >= 0)
    {
        network_server_close(server_socket);
    }

    if (car_list != NULL)
    {
        list_skel_destroy(car_list);
    }

    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <porto>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 8080\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Erro: porto inválido (%d). Use um valor entre 1 e 65535.\n", port);
        return 1;
    }

    // Registar handler para SIGINT
    signal(SIGINT, sigint_handler);

    // Inicializar lista
    car_list = list_skel_init();
    if (car_list == NULL)
    {
        fprintf(stderr, "Erro ao inicializar lista de carros.\n");
        return 1;
    }

    printf("Lista de carros inicializada.\n");

    // Inicializar servidor de rede
    server_socket = network_server_init(port);
    if (server_socket < 0)
    {
        fprintf(stderr, "Erro ao inicializar servidor na porta %d.\n", port);
        list_skel_destroy(car_list);
        return 1;
    }

    printf("Servidor iniciado com sucesso.\n");
    printf("A aguardar conexões... (Ctrl+C para terminar)\n\n");

    // Loop principal (não retorna a menos que haja erro)
    if (network_main_loop(server_socket, car_list) < 0)
    {
        fprintf(stderr, "Erro no loop principal do servidor.\n");
    }

    // Cleanup
    printf("\nA terminar servidor...\n");
    network_server_close(server_socket);
    list_skel_destroy(car_list);
    printf("Servidor terminado.\n");

    return 0;
}
