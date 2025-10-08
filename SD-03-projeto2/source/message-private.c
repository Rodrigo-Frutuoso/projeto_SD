/**
 * Funções auxiliares para leitura e escrita completa em sockets.
 *
 * Projeto: Sistemas Distribuídos 2025/2026
 */
#include <unistd.h>
#include <errno.h>
#include "message-private.h"

/* Escreve 'len' bytes de 'buf' no socket 'sock'.
 * Garante que todos os bytes são escritos, mesmo que write() retorne parcialmente.
 * Retorna 0 (OK) ou -1 (erro).
 */
int write_all(int sock, void *buf, int len)
{
    int written = 0;
    char *ptr = (char *)buf;

    while (written < len)
    {
        int result = write(sock, ptr + written, len - written);
        if (result <= 0)
        {
            if (result == 0 || errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            written += result;
        }
    }

    return 0;
}

/* Lê 'len' bytes do socket 'sock' para 'buf'.
 * Garante que todos os bytes são lidos, mesmo que read() retorne parcialmente.
 * Retorna 0 (OK) ou -1 (erro).
 */
int read_all(int sock, void *buf, int len)
{
    int total_read = 0;
    char *ptr = (char *)buf;

    while (total_read < len)
    {
        int result = read(sock, ptr + total_read, len - total_read);
        if (result <= 0)
        {
            if (result == 0 || errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            total_read += result;
        }
    }

    return 0;
}
