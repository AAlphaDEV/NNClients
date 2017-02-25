#ifndef USER_IN_H
#define USER_IN_H

#include "utils.h"
#include "recv.h"

#define COMMANDS 10

typedef struct {
    char cmd_name[10];
    int(*cmd_func)(SOCKET *s, int argc, char **argv);
} cmd_func_t;

cmd_func_t cmd_funcs[COMMANDS];

int usr_run;

char curr_path[512];
char last_path[512];

FILE *opened_file;

void init_cmds();
void wait_input(SOCKET *s);
void analyse_input(char *input, int cmd_name_len, char **args, int *out_arg_count, int max_args);

/******** Command funcs ********/
int cmd_ls(SOCKET *s, int argc, char **args);
int cmd_cd(SOCKET *s, int argc, char **args);
int cmd_exit(SOCKET *s, int argc, char **args);
int cmd_stop(SOCKET *s, int argc, char **args);
int cmd_ping(SOCKET *s, int argc, char **args);
int cmd_send(SOCKET *s, int argc, char **args);
int cmd_get(SOCKET *s, int argc, char **args);
int cmd_del(SOCKET *s, int argc, char **args);
int cmd_regs(SOCKET *s, int argc, char **args);
int cmd_rege(SOCKET *s, int argc, char **args);

void res_cmd_cd(SOCKET *s, char *argv[], void *arg);
void res_cmd_ls(SOCKET *s, char *argv[], void *arg);
void res_cmd_get(SOCKET *s, char *argv[], void *arg);
void res_cmd_regs(SOCKET *s, char *argv[], void *arg);

void get_path(const char *input, char *path);
void show_subfile(SOCKET *s, char *subfile, char *path);
/*******************************/

#endif // USER_IN_H
