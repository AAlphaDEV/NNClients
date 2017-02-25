#ifndef UTILS_H
#define UTILS_H

/**
    NNClient
**/

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>

void nnuc_exit();

void copy_file(FILE *source, FILE *dest);
int strindex(char *str, int c);

void fatal(char *err_msg);
void fatalWS(char *err_msg);

void safe_pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
void safe_pthread_join(pthread_t t, void **res);

void safe_send(SOCKET s, const char* b, int len, int flags);

#endif // UTILS_H
