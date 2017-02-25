#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <errno.h>
#include <time.h>

#include "utils.h"
#include "req_action.h"

void wait_input(SOCKET s);

void *recv_thread(void *arg);
void analyse_recv(SOCKET s, char *buffer);
void send_chars();
void update_action();
void recv_file(char *name, char *open_mode, SOCKET s, int nb_parts);

int is_running();
int is_accepted();

/***************************/

pthread_mutex_t mx_running;
int running;

pthread_mutex_t mx_accepted;
int accepted;

int resay;

int client_id;
int PORT;
char IP[24];

#endif // CLIENT_H
