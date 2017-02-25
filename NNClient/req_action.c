/**
    NNClient
**/

#include "req_action.h"

void request_action(SOCKET s, int action)
{
    char buffer[128];
    char a_name[10];
    int args;

    if(action == -1)
    {
        printf("\tChoose action :\n");
        printf("1. action_ls\n");
        printf("2. action_start\n");
        printf("3. action_delf\n");
        printf("4. action_sendf\n");
        printf("5. action_log\n");
        printf("6. action_errlog\n");
        printf("7. action_stat\n");

        printf("> ");
        fgets(buffer, 10, stdin);
        if(strlen(buffer) > 0 && buffer[strlen(buffer)-1] == '\n')
            buffer[strlen(buffer)-1] = '\0';
    } else
    {
        sprintf(buffer, "%d", action);
    }

    printf("Well, let's do action %s.\n", buffer);
    if(strcmp(buffer, "1") == 0)
    {
        //action_ls
        strcpy(a_name, "ls");
        args = 1;
        request(s, a_name, args);
    } else if(strcmp(buffer, "2") == 0)
    {
        //action_start
        strcpy(a_name, "start");
        args = 2;
        request(s, a_name, args);
    } else if(strcmp(buffer, "3") == 0)
    {
        //action_delf
        strcpy(a_name, "delf");
        args = 1;
        request(s, a_name, args);
    } else if(strcmp(buffer, "4") == 0)
    {
        //action_sendf
        strcpy(a_name, "sendf");
        args = 1;
        request(s, a_name, args);
    } else if(strcmp(buffer, "5") == 0)
    {
        //action_log
        strcpy(a_name, "log");
        args = 0;
        request(s, a_name, args);
    } else if(strcmp(buffer, "6") == 0)
    {
        //action_errlog
        strcpy(a_name, "errlog");
        args = 0;
        request(s, a_name, args);
    } else if(strcmp(buffer, "7") == 0)
    {
        //action_stat
        strcpy(a_name, "stat");
        args = 1;
        request(s, a_name, args);
    } else
    {
        printf("There is no action %s, you're a funny guy.\n", buffer);
        return;
    }

    printf("Waiting for results...\n");
    wait_res();
}

void request(SOCKET s, char *name, int args)
{
    char buffer[512];
    char arg_buf[128];

    strcpy(buffer, name);
    strcat(buffer, "/");

    int i;
    for(i = 0; i<args; i++)
    {
        printf("arg%d=", i);
        fgets(arg_buf, 128, stdin);

        if((strlen(arg_buf)>0) && (arg_buf[strlen(arg_buf)-1] == '\n'))
            arg_buf[strlen(arg_buf)-1] = '\0';

        strcat(buffer, arg_buf);
        strcat(buffer, "/");

        fflush(stdin);
    }
    strcat(buffer, "/");

    printf("Sending request (\"%s\")...\n", buffer);

    safe_send(s, buffer, strlen(buffer), 0);
}

int is_waiting_res()
{
    pthread_mutex_lock(&mx_waiting_res);
    int r = waiting_res;
    pthread_mutex_unlock(&mx_waiting_res);

    return r;
}
void wait_res()
{
    pthread_mutex_lock(&mx_waiting_res);
    waiting_res = 1;
    pthread_mutex_unlock(&mx_waiting_res);

    while(is_waiting_res())
    {
        Sleep(50);
    }
}

