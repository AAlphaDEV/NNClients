#include "user_in.h"

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int cmd_ls(SOCKET *s, int argc, char **args)
{
    char path[512];
    char buffer[512];

    if(argc < 1)
    {
        if(strlen(curr_path) == 0)
        {
            printf("Current path is not set.\n");
            return 3;
        }
        strcpy(path, curr_path);
    } else
    {
        if((args[0][strlen(args[0])-1] != '\\') && (args[0][strlen(args[0])-1] != '/'))
        {
            strcat(args[0], "\\");
        }

        get_path(args[0], path);
    }

    strcpy(last_path, path);

    expected_action_t a;
    strcpy(a.action, "action_ls");
    a.res_func = res_cmd_ls;

    set_expected_action(&a);
    wait_recv(1);

    sprintf(buffer, "ls/%s//", path);
    safe_send(*s, buffer, strlen(buffer), 0);

    while(is_waiting_recv())
    {
        Sleep(50);
    }

    return 0;
}

void show_subfile(SOCKET *s, char *subfile, char *path)
{
    char buffer[512];
    char buffer2[512];
    int rsize;
    int stat_sz;
    struct stat recv_stat;

    strcpy(buffer2, path);
    strcat(buffer2, subfile);

    sprintf(buffer, "stat/%s//", buffer2);
    safe_send(*s, buffer, strlen(buffer), 0);

    rsize = recv(*s, buffer, 512, 0);
    buffer[rsize] = '\0';

    sscanf(buffer, "%s//", buffer2);
    if(strcmp(buffer2, "failed") == 0)
    {
        printf("\t                \t%s\n", subfile);
        return;
    }
    stat_sz = atoi(buffer2);

    rsize = recv(*s, buffer, stat_sz, 0);
    if(rsize != stat_sz)
    {
        printf("\t                \t%s\n", subfile);
        return;
    }

    memcpy(&recv_stat, buffer, stat_sz);

    if(S_ISDIR(recv_stat.st_mode))
    {
        strcat(subfile, "/");
    }

    struct tm *time;
    time = localtime(&recv_stat.st_mtime);
    strftime(buffer2, 512, "\t%d/%m/%Y %H:%M", time);
    printf("%s\t%s\n", buffer2, subfile);
}

void res_cmd_ls(SOCKET *s, char *argv[], void *arg)
{
    if(argv[1] == NULL)
    {
        return;
    }

    char buffer[1024];
    char buffer2[512];
    char *token;

    printf("Repertory : %s\n", last_path);

    sscanf(argv[1], "subfiles=%[^\t\n]", buffer);

    printf("\t LastWriteTime   \tName\n");
    printf("\t-----------------\t-----\n");

    if(strchr(buffer, ';') == NULL)
    {
        sscanf(buffer, "\"%[^\t\n\"]\"", buffer2);
        show_subfile(s, buffer2, last_path);
    } else
    {
        token = strtok(buffer, ";");
        while(token != NULL)
        {
            sscanf(token, "\"%[^\t\n\"]\"", buffer2);
            show_subfile(s, buffer2, last_path);

            token = strtok(NULL, ";");
        }
    }
}

int cmd_cd(SOCKET *s, int argc, char **args)
{
    if(argc < 1)
    {
        printf("Usage : cd <path>\n");
        return 1;
    }

    char buffer[512];
    char path[512];

    if(strcmp(args[0], "..") == 0)
    {
        strcpy(buffer, curr_path);
        buffer[strlen(buffer)-1] = '\0';
        int last_slash = -1;
        int i;
        for(i = 0; i<strlen(buffer); i++)
        {
            if(buffer[i] == '\\')
            {
                last_slash = i;
            }
        }
        if(last_slash != -1)
        {
            buffer[last_slash+1] = '\0';
            strcpy(path, buffer);
        } else
        {
            strcpy(path, curr_path);
        }
    } else
    {
        if((args[0][strlen(args[0])-1] != '\\') && (args[0][strlen(args[0])-1] != '/'))
        {
            strcat(args[0], "\\");
        }

        if(strchr(args[0], '/') != NULL)
        {
            int i;
            for(i = 0; i<strlen(args[0]); i++)
            {
                if(args[0][i] == '/')
                    args[0][i] = '\\';
            }
        }

        if(strchr(args[0], ':') == NULL)
        {
            strcpy(path, curr_path);
            strcat(path, args[0]);
        } else
        {
            strcpy(path, args[0]);
        }
    }

    expected_action_t a;
    strcpy(a.action, "action_dir");
    a.res_func = res_cmd_cd;

    set_expected_action(&a);
    wait_recv(1);

    sprintf(buffer, "dir/%s//", path);
    safe_send(*s, buffer, strlen(buffer), 0);

    while(is_waiting_recv())
    {
        Sleep(50);
    }
    if(get_wait_res() == 0)
    {
        printf("Folder \"%s\" doesn't exists.\n", path);
        return 2;
    }

    strcpy(curr_path, path);

    return 0;
}

void res_cmd_cd(SOCKET *s, char *argv[], void *arg)
{
    if(argv[1] == NULL)
    {
        return;
    }

    int exists = atoi(argv[1]);
    set_wait_res(exists);
}

int cmd_send(SOCKET *s, int argc, char **args)
{
    if(argc < 1)
    {
        printf("Usage : send <data-to-sent>\n");
        return 1;
    }

    set_wait_res(6);
    wait_recv(1);

    safe_send(*s, args[0], strlen(args[0]), 0);
    while(is_waiting_recv())
    {
        Sleep(10);
    }

    return 0;
}

int cmd_get(SOCKET *s, int argc, char **args)
{
    if(argc < 1)
    {
        printf("Usage : get <path>\n");
        return 1;
    }

    char buffer[512];
    char path[512];
    char filename[512];

    memset(path, 0, 512);

    get_path(args[0], path);

    printf("Getting file \"%s\"...\n", args[0]);

    if(strchr(path, '\\') != NULL)
    {
        int last_slash;
        int i;
        for(i = 0; i<strlen(path); i++)
        {
            if(path[i] == '\\')
            {
                last_slash = i;
            }
        }

        strcpy(filename, path+last_slash+1);
    } else
    {
        strcpy(filename, path);
    }

    opened_file = fopen(filename, "wb");
    if(opened_file == NULL)
    {
        printf("Failed to open file \"%s\" : %s (errcode=%d)\n", args[0], strerror(errno), errno);
        return 2;
    }

    expected_action_t action;
    action.res_func = res_cmd_get;
    strcpy(action.action, "action_sendf");

    set_wait_res(7);
    set_expected_action(&action);
    wait_recv(1);

    sprintf(buffer, "sendf/%s//", path);
    safe_send(*s, buffer, strlen(buffer), 0);
    while(is_waiting_recv())
    {
        Sleep(20);
    }
    if(get_wait_res() == 7 && opened_file != NULL)
    {
        fclose(opened_file);
        remove(filename);
    }

    return 0;
}

int cmd_regs(SOCKET *s, int argc, char **args)
{
    if(argc < 1)
    {
        printf("Usage : regs <registry key>\n");
        return 1;
    }

    char buffer[512];
    sprintf(buffer, "regs/%s//", args[0]);

    expected_action_t action;
    strcpy(action.action, "action_regs");
    action.res_func = res_cmd_regs;
    action.arg = args[0];

    set_expected_action(&action);
    wait_recv(1);

    safe_send(*s, buffer, strlen(buffer), 0);

    while(is_waiting_recv())
    {
        Sleep(20);
    }

    return 0;
}

void res_cmd_regs(SOCKET *s, char *argv[], void *arg)
{
    if(argv[0] == NULL || argv[1] == NULL)
    {
        printf("Error : not enough arguments.\n");
        return;
    }

    char buffer[2048];
    char buffer2[1024];
    char val_name[128];
    char *token;
    int rsize;
    int is_values = 1;

    if(argv[2] == NULL)
    {
        is_values = 0;
    }

    strcpy(buffer, argv[1]);

    printf("%s\n", (char *) arg);
    if(strchr(buffer, ';') == NULL)
    {
        sscanf(buffer, "\"%[^\t\n\"]\"", buffer2);
        printf("   -> %s\n", buffer2);
    } else
    {
        token = strtok(buffer, ";");
        while(token != NULL)
        {
            sscanf(token, "\"%[^\t\n\"]\"", buffer2);
            printf("   -> %s\n", buffer2);

            token = strtok(NULL, ";");
        }
    }

    if(is_values)
    {
        strcpy(buffer, argv[2]);
        printf("Values :\n");
        if(strchr(buffer, ';') == NULL)
        {
            sscanf(token, "\"%[^\t\n\"]\"", buffer2);
            strcpy(val_name, buffer2);

            sprintf(buffer2, "regv/%s/%s//", (char *) arg, val_name);
            safe_send(*s, buffer2, strlen(buffer2), 0);
            rsize = recv(*s, buffer2, 1024, 0);
            buffer2[rsize] = '\0';

            char id[10];
            char *recv_args[3];
            get_recv_args(buffer2, id, recv_args);
            if(strcmp(id, "result") == 0)
            {
                printf("   - %s : '%s'", val_name, recv_args[2]);
            }
        } else
        {
            int buf_len = strlen(buffer);
            int index = strindex(buffer, ';');
            buffer[index] = '\0';
            token = buffer;
            while(token < (buffer+buf_len))
            {
                sscanf(token, "\"%[^\t\n\"]\"", buffer2);
                strcpy(val_name, buffer2);

                sprintf(buffer2, "regv/%s/%s//", (char *) arg, val_name);
                safe_send(*s, buffer2, strlen(buffer2), 0);
                rsize = recv(*s, buffer2, 1024, 0);
                buffer2[rsize] = '\0';

                char id[10];
                char *recv_args[3];
                get_recv_args(buffer2, id, recv_args);
                if(strcmp(id, "result") == 0)
                {
                    printf("   - %s : '%s'\n", val_name, recv_args[2]);
                }

                if(strindex(buffer+index+1, ';') == -1)
                {
                    token = buffer+index+1;
                    continue;
                }

                token = buffer+index+1;
                index += strindex(buffer+index+1, ';') + 1;
                buffer[index] = '\0';
            }
        }
    }
}

int cmd_rege(SOCKET *s, int argc, char **args)
{
    if(argc < 3)
    {
        printf("Usage : rege <registry key> <value's name> <value>\n");
        return 1;
    }

    char buffer[1024];

    set_wait_res(3);
    wait_recv(1);

    sprintf(buffer, "rege/%s/%s/%s//", args[0], args[1], args[2]);
    safe_send(*s, buffer, strlen(buffer), 0);

    while(is_waiting_recv())
    {
        Sleep(50);
    }

    return 0;
}

#define PART_SZ 65536

void res_cmd_get(SOCKET *s, char *argv[], void *arg)
{
    char buffer[PART_SZ];
    int rsize;
    int nb_parts = 0;

    if(opened_file == NULL)
    {
        printf("Error : file not opened.\n");
        return;
    }

    nb_parts = atoi(argv[1]);

    Sleep(50);
    int i;
    for(i = 0; i<nb_parts; i++)
    {
        rsize = recv(*s, buffer, PART_SZ, 0);

        printf(".");

        int j;
        for(j = 0; j<rsize; j++)
        {
            fputc(buffer[j], opened_file);
        }
    }

    printf("\n");

    fclose(opened_file);
    opened_file = NULL;
    set_wait_res(0);
}

int cmd_del(SOCKET *s, int argc, char **args)
{
    if(argc < 1)
    {
        printf("Usage : del <path>\n");
        return 1;
    }

    char buffer[512];
    char path[512];

    get_path(args[0], path);

    printf("Deleting file '%s'...\n", path);

    wait_recv(1);

    sprintf(buffer, "delf/%s//", path);
    safe_send(*s, buffer, strlen(buffer), 0);

    while(is_waiting_recv())
    {
        Sleep(20);
    }

    return 0;
}

int cmd_ping(SOCKET *s, int argc, char **args)
{
    int nb_packet = 5;
    char buffer[512];
    time_t timer;
    long int ellapsed = 0;
    int ping;

    if(argc >= 1)
    {
        nb_packet = atoi(args[0]);
        if(nb_packet == 0)
            return 1;
    }

    printf("Sending %d ping request to server...\n", nb_packet);

    strcpy(buffer, "ping//");
    int i;
    for(i = 0; i<nb_packet; i++)
    {
        wait_recv(1);
        timer = time(NULL);
        safe_send(*s, buffer, strlen(buffer), 0);
        while(is_waiting_recv())
        {}
        ellapsed += time(NULL) - timer;
    }

    ping = ellapsed / nb_packet;
    printf("Ping average : %d ms.\n", ping);

    return 0;
}

int cmd_exit(SOCKET *s, int argc, char **args)
{
    nnuc_exit(s);
    return 0;
}

int cmd_stop(SOCKET *s, int argc, char **args)
{
    char buffer[512];
    strcpy(buffer, "stop//");

    safe_send(*s, buffer, strlen(buffer), 0);
    nnuc_exit(s);

    return 0;
}
