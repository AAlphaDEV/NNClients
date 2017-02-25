#ifndef UTILS_H
#define UTILS_H

/**
    NNClient
**/

/**** HEADER FOR GETADDRINFO AND FREEADDRINFO ****/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef __cplusplus
extern "C" {
#endif
   void WSAAPI freeaddrinfo( struct addrinfo* );

   int WSAAPI getaddrinfo( const char*, const char*, const struct addrinfo*,
                 struct addrinfo** );

   int WSAAPI getnameinfo( const struct sockaddr*, socklen_t, char*, DWORD,
                char*, DWORD, int );
#ifdef __cplusplus
}
#endif
/**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void copy_file(FILE *source, FILE *dest);

void fatal(char *err_msg);
void fatalWS(char *err_msg);

void safe_pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
void safe_pthread_join(pthread_t t, void **res);

void safe_send(SOCKET s, const char* b, int len, int flags);

#endif // UTILS_H
