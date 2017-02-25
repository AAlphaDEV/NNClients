#include "utils.h"

void copy_file(FILE *source, FILE *dest)
{
    while(1)
    {
        int c = fgetc(source);
        if(c == EOF)
        {
            break;
        }
        fputc(c, dest);
    }
}

int strindex(char *str, int c)
{
    char *s = strchr(str, c);
    return (s == NULL) ? -1 : (s - str);
}

void fatal(char *err_msg)
{
    char error[128];
    strcpy(error, "[!!] Fatal Error ");
    strncat(error, err_msg, 105);

    perror(error);
    nnuc_exit();
}

void fatalWS(char *err_msg)
{
    char error[128];
    strcpy(error, "[!!] Fatal Error ");
    strncat(error, err_msg, 105);
    strcat(error, ": ");

    int lasterror = WSAGetLastError();
    char err_id[10];
    sprintf(err_id, "[%d] ", lasterror);
    strcat(error, err_id);

    char *s = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, lasterror,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                   (LPSTR)&s, 0, NULL);
    strcat(error, s);
    LocalFree(s);

    fprintf(stderr, "%s", error);
    nnuc_exit();
}

void safe_pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg)
{
    if(pthread_create(th, attr, func, arg) != 0)
    {
        fatal("while creating new thread ");
    }
}

void safe_pthread_join(pthread_t t, void **res)
{
    if(pthread_join(t, res) != 0)
    {
        fatal("while joining thread ");
    }
}

void safe_send(SOCKET s,const char* b , int len,int flags)
{
    if(send(s, b, len, flags) == -1)
    {
        fatalWS("while sending data ");
    }
}
