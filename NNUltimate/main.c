#include <stdio.h>

#include <windows.h>
#include <winsock2.h>

#include "utils.h"
#include "recv.h"
#include "user_in.h"

char IP[64];
unsigned int PORT;
SOCKET *main_sock;

int main(int argc, char *argv[])
{
    /***********************************************************************/
    printf("NoName Ultimate Client.\n\n");
    printf("$$\\   $$\\ $$\\   $$\\       $$\\   $$\\  $$$$$$\\   \n");
    printf("$$$\\  $$ |$$$\\  $$ |      $$ |  $$ |$$  __$$\\     \n");
    printf("$$$$\\ $$ |$$$$\\ $$ |      $$ |  $$ |$$ /  \\__|   \n");
    printf("$$ $$\\$$ |$$ $$\\$$ |      $$ |  $$ |$$ |           \n");
    printf("$$ \\$$$$ |$$ \\$$$$ |      $$ |  $$ |$$ |      \n");
    printf("$$ |\\$$$ |$$ |\\$$$ |      $$ |  $$ |$$ |  $$\\    \n");
    printf("$$ | \\$$ |$$ | \\$$ |      \\$$$$$$  |\\$$$$$$  |    \n");
    printf("\\__|  \\__|\\__|  \\__|       \\______/  \\______/  \n");
    printf("\n");

    printf("   _   _       _                  ___           \n");
    printf("  /_\\ | |_ __ | |__   __ _       /   \\_____   __\n");
    printf(" //_\\\\| | '_ \\| '_ \\ / _` |     / /\\ / _ \\ \\ / /\n");
    printf("/  _  \\ | |_) | | | | (_| |    / /_//  __/\\ V / \n");
    printf("\\_/ \\_/_| .__/|_| |_|\\__,_|___/___,' \\___| \\_/  \n");
    printf("        |_|              |_____|              \n");
    printf("\n");
    printf("\n");
    /***********************************************************************/

    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in addr;
    char buffer[512];
    int running;
    pthread_t recv_th;

    printf("Initializing network module...\n");
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fatalWS("while initializing winsock ");
    }

    printf("Creating socket...\n");
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

            WSACleanup();

            printf("\nPress ENTER to exit...");
            getchar();

            exit(0);
        }
        printf("Successfully connected to server.\n");
        break;
    }

    main_sock = &s;

    printf("Starting receiver thread...\n");

    recv_run_mutex = PTHREAD_MUTEX_INITIALIZER;
    waiting_recv_mutex = PTHREAD_MUTEX_INITIALIZER;
    wait_result_mutex = PTHREAD_MUTEX_INITIALIZER;
    expected_action_mutex = PTHREAD_MUTEX_INITIALIZER;
    wait_recv(1);

    safe_pthread_create(&recv_th, NULL, recv_thread, &s);

    while(is_waiting_recv())
    {
        Sleep(100);
    }

    if(get_wait_res() != 1)
    {
        nnuc_exit();
    }

    printf("Starting user console...\n");
    init_cmds();

    printf("\n");
    wait_input(&s);

    nnuc_exit();

    return 0;
}

void nnuc_exit()
{
    set_recv_running(0);
    Sleep(200);

    closesocket(*main_sock);
    WSACleanup();

    printf("\nPress ENTER to exit...");
    getchar();

    exit(0);
}
