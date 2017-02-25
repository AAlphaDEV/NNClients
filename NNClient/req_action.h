#ifndef REQ_ACTION_H
#define REQ_ACTION_H

/**
    NNClient
**/

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#include "utils.h"

pthread_mutex_t mx_waiting_res;
int waiting_res;

void request_action(SOCKET s, int action);

void request(SOCKET s, char *name, int args);
int is_waiting_res();
void wait_res();

#endif // REQ_ACTION_H
