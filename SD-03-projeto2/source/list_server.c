/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "network_server.h"
#include "list_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/**
 * Programa principal do servidor.
 * Uso: ./list_server <porto>
 * 
 * O servidor:
 * 1. Valida argumentos da linha de comandos
 * 2. Inicializa a lista de carros (skeleton)
 * 3. Cria um socket TCP e fica à escuta na porta especificada
 * 4. Entra num loop infinito atendendo clientes
 * 5. Para cada cliente: recebe pedido, processa, envia resposta
 */
int main(int argc, char **argv) {
    struct list_t *list;
    int listening_socket;
    short port;

    /* ========================================================================
     * 1. VALIDAR ARGUMENTOS
     * ======================================================================== */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <porto>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 12345\n", argv[0]);
        return -1;
    }

    /* ========================================================================
     * 2. CONVERTER PORTA PARA NÚMERO
     * ======================================================================== */
    port = (short) atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Erro: porta inválida '%s'\n", argv[1]);
        fprintf(stderr, "A porta deve ser um número entre 1 e 65535\n");
        return -1;
    }

    /* ========================================================================
     * 3. IGNORAR SIGPIPE
     * 
     * SIGPIPE é enviado quando tentamos escrever num socket já fechado.
     * Sem esta linha, o servidor crasharia quando um cliente desconectasse
     * abruptamente. Com SIG_IGN, o write() retorna erro mas não crasha.
     * ======================================================================== */
    signal(SIGPIPE, SIG_IGN);

    /* ========================================================================
     * 4. INICIALIZAR SKELETON (criar lista vazia de carros)
     * ======================================================================== */
    list = list_skel_init();
    if (list == NULL) {
        fprintf(stderr, "Erro: falha ao inicializar a lista de carros\n");
        return -1;
    }

    printf("✓ Lista de carros inicializada\n");

    /* ========================================================================
     * 5. INICIALIZAR SERVIDOR DE REDE
     * 
     * Cria socket TCP, faz bind() à porta, e coloca em modo listen().
     * Retorna o descritor do socket ou -1 em erro.
     * ======================================================================== */
    listening_socket = network_server_init(port);
    if (listening_socket < 0) {
        fprintf(stderr, "Erro: falha ao inicializar servidor na porta %d\n", port);
        list_skel_destroy(list);
        return -1;
    }

    printf("✓ Servidor a ouvir na porta %d\n", port);
    printf("✓ Aguardando conexões de clientes...\n");
    printf("  (Prima Ctrl+C para terminar)\n\n");

    /* ========================================================================
     * 6. LOOP PRINCIPAL
     * 
     * Esta função NÃO retorna em condições normais (loop infinito).
     * 
     * Dentro dela:
     * - accept() espera por clientes
     * - network_receive() recebe mensagem protobuf
     * - invoke() processa pedido na lista
     * - network_send() envia resposta
     * - close() fecha conexão com cliente
     * 
     * Só retorna -1 se houver erro grave (ex: socket corrompido).
     * ======================================================================== */
    if (network_main_loop(listening_socket, list) < 0) {
        fprintf(stderr, "Erro: falha no loop principal do servidor\n");
    }

    /* ========================================================================
     * 7. LIMPEZA DE RECURSOS
     * 
     * Código só executado se:
     * - network_main_loop() retornar com erro
     * - Sinal de interrupção (Ctrl+C, kill, etc.)
     * ======================================================================== */
    printf("\nEncerrando servidor...\n");
    
    network_server_close(listening_socket);
    printf("✓ Socket fechado\n");
    
    list_skel_destroy(list);
    printf("✓ Lista destruída e memória libertada\n");
    
    printf("Servidor terminado.\n");

    return 0;
}
