#include "client.h"

void wait_input(SOCKET s)
{
    char buffer[512];

    resay = 1;
    while(is_running())
    {
        if(resay)
        {
            printf("\tChoose something to do :\n");
            printf("send : Send characters to server (Advanced)\n");
            printf("action : Request action\n");
            printf("up : Update\n");
            printf("stop : Send stop signal and quit\n");
            printf("quit : Just quit\n");
            printf("Or use aliases\n");
        }

        printf("> ");
        fgets(buffer, 512, stdin);

        if((strlen(buffer)>0) && (buffer[strlen(buffer)-1] == '\n'))
            buffer[strlen(buffer)-1] = '\0';

        if(is_running() == 0)
            break;

        if(strcmp(buffer, "send") == 0)
        {
            //Send chars
            send_chars(s);
        } else if(strcmp(buffer, "action") == 0)
        {
            //Request action
            request_action(s, -1);
        } else if(strcmp(buffer, "up") == 0)
        {
            //update
            update_action(s);
        } else if(strcmp(buffer, "ls") == 0)
        {
            //Alias for action_ls
            request_action(s, 1);
        } else if(strcmp(buffer, "quit") == 0)
        {
            //quit
            pthread_mutex_lock(&mx_running);
            running = 0;
            pthread_mutex_unlock(&mx_running);
        } else if(strcmp(buffer, "stop") == 0)
        {
            //send stop signal
            strcpy(buffer, "stop");
            safe_send(s, buffer, strlen(buffer), 0);

            pthread_mutex_lock(&mx_running);
            running = 0;
            pthread_mutex_unlock(&mx_running);
        } else
        {
            if(!is_running())
                break;
            printf("That wasn't in the choices.\n");
        }

        fflush(stdin);
        resay = 0;
    }
}

void send_chars(SOCKET s)
{
    char buffer[512];
    printf("Let's send what you want to server.\n");
    printf("buffer=");

    fgets(buffer, 512, stdin);

    if((strlen(buffer)>0) && (buffer[strlen(buffer)-1] == '\n'))
        buffer[strlen(buffer)-1] = '\0';

    printf("Sending data...\n");

    safe_send(s, buffer, strlen(buffer), 0);
}

int is_running()
{
    pthread_mutex_lock(&mx_running);
    int r = running;
    pthread_mutex_unlock(&mx_running);
    return r;
}

int is_accepted()
{
    pthread_mutex_lock(&mx_accepted);
    int r = accepted;
    pthread_mutex_unlock(&mx_accepted);
    return r;
}

void *recv_thread(void *arg)
{
    SOCKET *cl = (SOCKET *) arg;
    char buffer[4096];
    int len;

    while(is_running())
    {
        memset(buffer, 0, 4096);
        len = recv(*cl, buffer, 4096, 0);
        if(len == 0 || len == -1)
        {
            printf("\nConnection to server lost.\n");
            printf("Press enter to quit...");

            if(is_waiting_res())
            {
                pthread_mutex_lock(&mx_waiting_res);
                waiting_res = 0;
                pthread_mutex_unlock(&mx_waiting_res);
                printf("No result received.\n");
            }

            pthread_mutex_lock(&mx_running);
            running = 0;
            pthread_mutex_unlock(&mx_running);

            pthread_exit(NULL);
            return NULL;
        }
        //printf("\n\tlen=%d\n", len);
        //printf("\treceived=\"%s\"\n\n", buffer);

        buffer[len] = '\0';

        analyse_recv(*cl, buffer);
    }

    pthread_exit(NULL);
    return NULL;
}

#define PART_SZ 65536

void recv_file(char *name, char *open_mode, SOCKET s, int nb_parts)
{
    FILE *file;
    char buffer[PART_SZ];
    int rsize;
    time_t timer;
    long int ellapsed;

    timer = time(NULL);

    file = fopen(name, open_mode);
    int i;
    for(i = 0; i<nb_parts; i++)
    {
        rsize = recv(s, buffer, PART_SZ, 0);
        printf("\tWriting part %d\n", i);
        int j;
        for(j = 0; j<rsize; j++)
        {
            fputc((int) buffer[j], file);
        }
    }
    fclose(file);

    ellapsed = time(NULL) - timer;

    printf("File writed successfully.\n");
    printf("Ellapsed time : %ld s.\n", ellapsed);
}
