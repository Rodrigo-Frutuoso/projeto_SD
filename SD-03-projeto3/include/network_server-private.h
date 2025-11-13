/**
 * Projeto: Sistemas Distribuídos 2025/2026
 * Autor: José Cecílio
 * Data: 4/10/2025
 */

#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include <pthread.h>

void *client_handler(void *args);
pthread_mutex_t* get_list_mutex();
int get_num_clientes_ativos();

#endif
