/**
    NNClient
**/

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utils.h"
#include "req_action.h"
#include "client.h"

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in addr;
    pthread_t recv_th;
    char buffer[512];

    printf("******************************\n");
    printf("****   NoName_v0 Client   ****\n");
    printf("****       Alpha_Dev      ****\n");
    printf("******************************\n");

    printf("Starting network module...\n");
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fatalWS("while initializing winsock ");
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == INVALID_SOCKET)
    {
        fatalWS("while creating socket ");
    }

    running = 1;
    while(running)
    {
        printf("Server IP : ");
        fgets(buffer, 512, stdin);
        if((strlen(buffer) > 0) && buffer[strlen(buffer)-1] == '\n')
            buffer[strlen(buffer)-1] = '\0';
        strcpy(IP, buffer);

        int p;
        while(1)
        {
            printf("Server port : ");
            fgets(buffer, 512, stdin);
            if((strlen(buffer) > 0) && buffer[strlen(buffer)-1] == '\n')
                buffer[strlen(buffer)-1] = '\0';
            p = atoi(buffer);
            if(p <= 0 || p > 65535)
            {
                printf("%d is an invalid port number.\n", p);
                continue;
            }
            break;
        }
        PORT = p;

        addr.sin_addr.s_addr = inet_addr(IP);
        addr.sin_port = htons(PORT);
        addr.sin_family = AF_INET;

        printf("Connecting to server at %s:%d...\n", IP, PORT);
        if(connect(s, (struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1)
        {
            printf("Failed to connect to server. (errcode=%d)\n", WSAGetLastError());
            printf("Retry ? (Y/N) ");

            fgets(buffer, 10, stdin);
            if((strlen(buffer) > 0) && buffer[strlen(buffer)-1] == '\n')
                buffer[strlen(buffer)-1] = '\0';
            if(strcmp(buffer, "Y") == 0 || strcmp(buffer, "y") == 0)
            {
                continue;
            }

            printf("Goodbye.\n");
            WSACleanup();
            return 0;
        }
        printf("Successfully connected to server.\n");
        break;
    }

    printf("Starting receiver thread...\n");
    safe_pthread_create(&recv_th, NULL, recv_thread, &s);

    mx_running = PTHREAD_MUTEX_INITIALIZER;
    mx_accepted = PTHREAD_MUTEX_INITIALIZER;
    mx_waiting_res = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mx_accepted);
    accepted = 0;
    pthread_mutex_unlock(&mx_accepted);

    pthread_mutex_lock(&mx_running);
    running = 1;
    pthread_mutex_unlock(&mx_running);

    while(!is_accepted())
    {
        if(!is_running())
            break;
        Sleep(50);
    }

    if(is_running())
    {
        printf("You were accepted by server (client_id=%d).\n", client_id);

        printf("All is ok, we can begin.\n\n");
    }

    wait_input(s);

    printf("Goodbye.\n");

    closesocket(s);
    WSACleanup();

    return 0;
}

void analyse_recv(SOCKET s, char *buffer)
{
    char buf[512];
    char buf2[1024];
    char id[10];
    char *token;
    char *args[3];

    printf("\tReceived : \"%s\"\n", buffer);

    int j;
    for(j = 0; j<3; j++)
    {
        args[j] = 0;
    }

    token = strtok(buffer, "/");
    if(strlen(token) > 10)
        return;
    strcpy(id, token);

    int i = 0;
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

    if(strcmp(id, "accepted") == 0)
    {
        if(i>0)
        {
            sscanf(args[0], "id=%d", &client_id);
        }

        pthread_mutex_lock(&mx_accepted);
        accepted = 1;
        pthread_mutex_unlock(&mx_accepted);
    } else if(strcmp(id, "refused") == 0)
    {
        if(i>0)
        {
            sscanf(args[0], "reason=\"%[^\t\n\"]\"", buf);
        }

        printf("[!!] You were refused by server, reason : \"%s\"\n", buf);

        pthread_mutex_lock(&mx_running);
        running = 0;
        pthread_mutex_unlock(&mx_running);
    } else if(strcmp(id, "result") == 0)
    {
        if(i<2)
        {
            return;
        }

        sscanf(args[0], "action=\"%[^\t\n\"]\"", buf);
        printf("Result from \"%s\" received : ", buf);

        if(strcmp(buf, "action_ls") == 0)
        {
            sscanf(args[1], "subfiles=%[^\t\n]", buf2);
            printf("raw : %s\n", buf2);
        } else if(strcmp(buf, "action_sendf") == 0)
        {
            printf("\n");
            int nb_parts = atoi(args[1]);
            recv_file("received.file", "wb", s, nb_parts);
            if(is_waiting_res())
            {
                pthread_mutex_lock(&mx_waiting_res);
                waiting_res = 0;
                pthread_mutex_unlock(&mx_waiting_res);
            }
            return;
        } else if(strcmp(buf, "action_log") == 0)
        {
            printf("\n");
            int nb_parts = atoi(args[1]);
            recv_file("winsec.log", "w", s, nb_parts);
            if(is_waiting_res())
            {
                pthread_mutex_lock(&mx_waiting_res);
                waiting_res = 0;
                pthread_mutex_unlock(&mx_waiting_res);
            }
            return;
        } else if(strcmp(buf, "action_log") == 0)
        {
            printf("\n");
            int nb_parts = atoi(args[1]);
            recv_file("error.log", "w", s, nb_parts);
            if(is_waiting_res())
            {
                pthread_mutex_lock(&mx_waiting_res);
                waiting_res = 0;
                pthread_mutex_unlock(&mx_waiting_res);
            }
            return;
        } else if(strcmp(buf, "action_stat") == 0)
        {
            printf("\n");
            char buf3[512];
            int stat_sz = atoi(args[1]);
            printf("stat_sz=%d\n", stat_sz);

            recv(s, buf3, stat_sz, 0);
            struct stat *recv_stat;
            recv_stat = (struct stat *) malloc(sizeof(struct stat));
            if(recv_stat == NULL)
            {
                fprintf(stderr, "Error : failed to allocate memory (\"%s\") (errcode=%d)\n", strerror(errno), errno);
            }
            memcpy((void *) recv_stat, (void *) buf3, stat_sz);

            printf("Stats : st_dev=%hd\nst_ino=%hd\nst_mode=%hx\nst_nlink=%hd\nst_uid=%hd\nst_gid=%hd\n",
                   recv_stat->st_dev, recv_stat->st_ino, recv_stat->st_mode, recv_stat->st_nlink, recv_stat->st_uid, recv_stat->st_gid);
            printf("Directory : %d\n", S_ISDIR(recv_stat->st_mode));

            if(is_waiting_res())
            {
                pthread_mutex_lock(&mx_waiting_res);
                waiting_res = 0;
                pthread_mutex_unlock(&mx_waiting_res);
            }
            return;
        } else
        {
            strcpy(buf2, args[1]);
        }

        strcpy(buf, "");
        if(strchr(buf2, ';') == NULL)
        {
            sscanf(buf2, "\"%[^\t\n\"]\"", buf);
            printf("%s", buf);
        } else
        {
            token = strtok(buf2, ";");
            sscanf(token, "\"%[^\t\n\"]\"", buf);
            printf("\n\t%s", buf);

            token = strtok(NULL, ";");
            while(token != NULL)
            {
                sscanf(token, "\"%[^\t\n\"]\"", buf);
                printf("\n\t%s", buf);

                token = strtok(NULL, ";");
            }
        }
        printf("\n");

        if(is_waiting_res())
        {
            pthread_mutex_lock(&mx_waiting_res);
            waiting_res = 0;
            pthread_mutex_unlock(&mx_waiting_res);
        }
    } else if(strcmp(id, "failed") == 0)
    {
        if(i<2)
        {
            return;
        }

        sscanf(args[0], "action=\"%[^\t\n\"]\"", buf);
        sscanf(args[1], "reason=\"%[^\t\n\"]\"", buf2);
        printf("\"%s\" failed, reason : %s\n", buf, buf2);

        if(is_waiting_res())
        {
            pthread_mutex_lock(&mx_waiting_res);
            waiting_res = 0;
            pthread_mutex_unlock(&mx_waiting_res);
        }
    }
}

void update_action(SOCKET s)
{
    char ult_buffer[4096];
    char buffer[128];

    char filepath[128];
    char remote_filename[64];
    int restart;

    FILE *file;
    int file_size;
    int nb_parts;

    printf("filepath=");
    fgets(filepath, 128, stdin);

    if((strlen(filepath)>0) && (filepath[strlen(filepath)-1] == '\n'))
        filepath[strlen(filepath)-1] = '\0';

    printf("remote filename=");
    fgets(remote_filename, 128, stdin);

    if((strlen(remote_filename)>0) && (remote_filename[strlen(remote_filename)-1] == '\n'))
        remote_filename[strlen(remote_filename)-1] = '\0';

    printf("restart (1 or 0)=");
    fgets(buffer, 128, stdin);

    if((strlen(buffer)>0) && (buffer[strlen(buffer)-1] == '\n'))
        buffer[strlen(buffer)-1] = '\0';

    restart = atoi(buffer);
    if(restart != 0 && restart != 1)
    {
        printf("\"restart\" must be 1 or 0\n");
        printf("\"restart\" set to 0 (default).\n");
        restart = 0;
    }

    printf("Getting file \"%s\"...\n", filepath);
    file = fopen(filepath, "rb");
    if(file == NULL)
    {
        fprintf(stderr, "Failed to open \"%s\", error : %s\n", filepath, strerror(errno));
        return;
    }

    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    nb_parts = (int) file_size / 4096;
    if(file_size % 4096 > 0)
        nb_parts++;

    printf("File size : %d - Number of parts : %d\n", file_size, nb_parts);

    sprintf(buffer, "up/%s/%d/%d//", remote_filename, nb_parts, restart);
    safe_send(s, buffer, strlen(buffer), 0);

    Sleep(50);

    int p;
    for(p = 0; p<nb_parts; p++)
    {
        Sleep(10);
        int ssize = 4096;
        int i;
        for(i = 0; i<4096; i++)
        {
            int c = fgetc(file);
            if(c == EOF)
            {
                printf("EOF - i=%d - c=%c (%d) - ftell()=%ld\n", i, c, c, ftell(file));
                ssize = i;
                break;
            }
            ult_buffer[i] = (char) c;
        }

        printf("Sending part %d - size : %d\n", p, ssize);
        safe_send(s, ult_buffer, ssize, 0);
    }

    fclose(file);

    printf("Waiting for results...\n");
    wait_res();
}

