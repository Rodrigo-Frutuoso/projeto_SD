/**
 * Grupo 03
 * Rodrigo Frutuoso - 61865
 * Simão Alexandre - 61874
 * Tiago Leite - 61863
 */

#include "message-private.h"
#include <unistd.h>
#include <errno.h>

int write_all(int sock, void *buf, int len) { //SLIDE 23 TP4. Sockets
    int bufsize = len;
    char *ptr = (char *)buf;
    
    while (len > 0) {
        int res = write(sock, ptr, len);
        if (res < 0) {
            if (errno == EINTR) continue;
            return res;
        }
        if (res == 0) return res;
        ptr += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, void *buf, int len) {
    int bufsize = len;
    char *ptr = (char *)buf;
    
    while (len > 0) {
        int res = read(sock, ptr, len);
        if (res < 0) {
            if (errno == EINTR) continue;
            return res;
        }
        if (res == 0) return res;
        ptr += res;
        len -= res;
    }
    return bufsize;
}
