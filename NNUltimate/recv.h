#ifndef RECV_H
#define RECV_H

#include "utils.h"
#include "user_in.h"

pthread_mutex_t recv_run_mutex;
int recv_run;

pthread_mutex_t waiting_recv_mutex;
int waiting_recv;
pthread_mutex_t wait_result_mutex;
int wait_result;
/****
*** 2 -> Show received str
*** 3 -> Show action success
*** 5 -> pong
*** 7 -> failed to receive file
*** -1 -> connection to server lost
****/

typedef struct {
    char action[64];
    void(*res_func)(SOCKET *s, char *argv[], void *arg);
    void *arg;
} expected_action_t;

pthread_mutex_t expected_action_mutex;
expected_action_t *expected_action;

void *recv_thread(void *arg);
int is_recv_running();
void set_recv_running(int run);
void analyse_recv(SOCKET *s, char *received);
int get_recv_args(char *received, char *id, char **args);

int is_waiting_recv();
void wait_recv(int waiting);

int get_wait_res();
void set_wait_res(int res);

void set_expected_action(expected_action_t *action);
expected_action_t *get_expected_action();

#endif // RECV_H
