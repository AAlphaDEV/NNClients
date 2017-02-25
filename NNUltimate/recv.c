#include "recv.h"

int recv_run = 0;

void *recv_thread(void *arg)
{
    SOCKET *s = (SOCKET *) arg;

    char buffer[4096];
    int len;

    expected_action = NULL;

    recv_run = 1;
    while(recv_run)
    {
        len = recv(*s, buffer, 4096, 0);
        if(len == -1)
        {
            fprintf(stderr, "Connection to server lost. (errcode=%d)\n", WSAGetLastError());

            if(is_waiting_recv())
            {
                wait_recv(0);
                set_wait_res(-1);
            }

            pthread_mutex_lock(&recv_run_mutex);
            recv_run = 0;
            pthread_mutex_unlock(&recv_run_mutex);
            break;
        }

        buffer[len] = '\0';

        if(get_wait_res() == 2)
        {
            printf("Received (raw) : \"%s\"\n", buffer);
            set_wait_res(0);
        }

        analyse_recv(s, buffer);

        Sleep(10);
    }

    pthread_exit(NULL);
    return NULL;
}

void analyse_recv(SOCKET *s, char *received)
{
    char buffer2[512];
    char *args[3];
    char id[10];
    int cl_id;

    if(strchr(received, '/') == NULL)
    {
        printf("Received (misunderstood) : \"%s\"\n", received);
        if(is_waiting_recv())
            wait_recv(0);
        return;
    }

    int i = get_recv_args(received, id, args);

    if(strcmp(id, "accepted") == 0)
    {
        printf("You were accepted by server.\n");

        if(i >= 2)
        {
            sscanf(args[0], "id=%d", &cl_id);
            printf("\tYour ID : %d\n", cl_id);
            sscanf(args[1], "version=\"%[^\t\n\"]\"", buffer2);
            printf("\tServer version : %s\n", buffer2);
        }

        set_wait_res(1);

        if(is_waiting_recv())
        {
            wait_recv(0);
        }
    } else if(strcmp(id, "refused") == 0)
    {
        if(i>0)
        {
            sscanf(args[0], "reason=\"%[^\t\n\"]\"", buffer2);
        }

        printf("You were refused by server, reason : \"%s\".\n", buffer2);
        set_wait_res(2);

        if(is_waiting_recv())
        {
            wait_recv(0);
        }
    } else if(strcmp(id, "result") == 0)
    {
        if(i<1)
        {
            return;
        }

        sscanf(args[0], "action=\"%[^\t\n\"]\"", buffer2);

        if(get_wait_res() == 3)
        {
            if(strcmp(args[1], "success") == 0)
            {
                printf("Action \"%s\" successful.\n", buffer2);
            }
        }

        //printf("Received result for \"%s\".\n", buffer2);

        //printf("is_waiting : %d | action==NULL : %d\n", is_waiting_recv(), (get_expected_action() == NULL));
        if(is_waiting_recv() && (get_expected_action() != NULL))
        {
            //printf("buffer2 : %s | action : %s | strcmp : %d\n", buffer2, get_expected_action()->action, strcmp(buffer2, get_expected_action()->action));
            if(strcmp(buffer2, get_expected_action()->action) == 0)
            {
                get_expected_action()->res_func(s, args, get_expected_action()->arg);
                set_expected_action(NULL);
            }
        }

        if(is_waiting_recv())
        {
            wait_recv(0);
        }
    } else if(strcmp(id, "doing") == 0)
    {
        if(i<1)
        {
            return;
        }

        sscanf(args[0], "action=\"%[^\t\n\"]\"", buffer2);

        //printf("Doing action \"%s\".\n", buffer2);
    } else if(strcmp(id, "failed") == 0)
    {
        if(i<1)
        {
            return;
        }

        sscanf(args[0], "action=\"%[^\t\n\"]\"", buffer2);

        printf("Action \"%s\" failed.\n", buffer2);

        if(i>=2)
        {
            sscanf(args[1], "reason=\"%[^\t\n\"]\"", buffer2);
            printf("Reason : \"%s\"\n", buffer2);
        }

        if(is_waiting_recv())
        {
            wait_recv(0);
        }
    } else if(strcmp(id, "pong") == 0)
    {
        if(i<1)
        {
            return;
        }

        if(is_waiting_recv())
        {
            set_wait_res(5);
            wait_recv(0);
        }
    }
}

int get_recv_args(char *received, char *id, char **args)
{
    char *token;

    token = strtok(received, "/");
    strncpy(id, token, 10);

    int i = 0;
    for(i = 0; i<3; i++)
        args[i] = NULL;
    i = 0;
    while(token != NULL)
    {
        token = strtok(NULL, "/");

        if(i>=3)
        {
            i = 3;
            break;
        }
        args[i] = token;
        i++;
    }

    return i;
}

int is_recv_running()
{
    pthread_mutex_lock(&recv_run_mutex);
    int r = recv_run;
    pthread_mutex_unlock(&recv_run_mutex);
    return r;
}

void set_recv_running(int run)
{
    pthread_mutex_lock(&recv_run_mutex);
    recv_run = run;
    pthread_mutex_unlock(&recv_run_mutex);
}

int is_waiting_recv()
{
    pthread_mutex_lock(&waiting_recv_mutex);
    int r = waiting_recv;
    pthread_mutex_unlock(&waiting_recv_mutex);

    return r;
}

void wait_recv(int waiting)
{
    pthread_mutex_lock(&waiting_recv_mutex);
    waiting_recv = waiting;
    pthread_mutex_unlock(&waiting_recv_mutex);
}

int get_wait_res()
{
    pthread_mutex_lock(&wait_result_mutex);
    int r = wait_result;
    pthread_mutex_unlock(&wait_result_mutex);

    return r;
}

void set_wait_res(int res)
{
    pthread_mutex_lock(&wait_result_mutex);
    wait_result = res;
    pthread_mutex_unlock(&wait_result_mutex);
}

void set_expected_action(expected_action_t *action)
{
    pthread_mutex_lock(&expected_action_mutex);
    expected_action = action;
    pthread_mutex_unlock(&expected_action_mutex);
}

expected_action_t *get_expected_action()
{
    pthread_mutex_lock(&expected_action_mutex);
    expected_action_t *a = expected_action;
    pthread_mutex_unlock(&expected_action_mutex);
    return a;
}
