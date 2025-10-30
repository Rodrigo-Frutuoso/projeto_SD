/**
 * Projeto: Sistemas Distribuídos 2025/2026
 * Autor: José Cecílio
 * Data: 4/10/2025
 */

#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <pthread.h>

/* Função executada pelas threads secundárias para atender cada cliente */
void *client_handler(void *args);

/* Função para obter o mutex da lista (usado em list_skel.c) */
pthread_mutex_t* get_list_mutex();

#endif
