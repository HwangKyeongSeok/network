#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024 //
#define NAME_SIZE 20
#define MAX_CLNT 256

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);
void send_private_msg(char* msg, int len, char* recipient_name, int sender_sock);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE];
pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);

        printf("Connected client IP: %s \n", inet_ntoa(clnt_addr.sin_addr));
    }

    close(serv_sock);
    return 0;
}

void* handle_clnt(void* arg)
{
    int clnt_sock = *((int*)arg);
    int str_len = 0;
    char msg[BUF_SIZE];
    char name_msg[NAME_SIZE + BUF_SIZE + 5];

    // 클라이언트 이름 받기
    char client_name[NAME_SIZE];
    str_len = read(clnt_sock, client_name, NAME_SIZE - 1);
    if (str_len == -1)
        error_handling("read() error!");
    client_name[str_len] = 0;

    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (clnt_socks[i] == clnt_sock) {
            strncpy(clnt_names[i], client_name, NAME_SIZE);
            is_new_client = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mutx);

    printf("Client name: %s\n", client_name);

    while ((str_len = read(clnt_sock, msg, sizeof(msg) - 1)) != 0)
    {
        msg[str_len] = 0;
        if (msg[0] == '@') {
            char* recipient_name = strtok(msg + 1, " ");
            char* private_msg = strtok(NULL, "");
            if (recipient_name && private_msg) {
                snprintf(name_msg, sizeof(name_msg), "[%s] %s", client_name, private_msg);
                send_private_msg(name_msg, strlen(name_msg), recipient_name, clnt_sock);
            }
        }
        else {
            snprintf(name_msg, sizeof(name_msg), "[%s] %s", client_name, msg);
            send_msg(name_msg, strlen(name_msg));
        }
    }

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
    {
        if (clnt_sock == clnt_socks[i])
        {
            while (i < clnt_cnt - 1)
            {
                clnt_socks[i] = clnt_socks[i + 1];
                strncpy(clnt_names[i], clnt_names[i + 1], NAME_SIZE);
                i++;
            }

            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);

    close(clnt_sock);
    return NULL;
}

void send_msg(char* msg, int len)
{
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}

void send_private_msg(char* msg, int len, char* recipient_name, int sender_sock)
{
    int i;
    int recipient_found = 0;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (strcmp(clnt_names[i], recipient_name) == 0) {
            write(clnt_socks[i], msg, len);
            recipient_found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutx);

    if (!recipient_found) {
        char error_msg[NAME_SIZE + 50];
        for (i = 0; i < clnt_cnt; i++) {
            if (clnt_socks[i] == sender_sock) {
                snprintf(error_msg, sizeof(error_msg), "User [%s] not found.\n", recipient_name);
                write(clnt_socks[i], error_msg, strlen(error_msg));
                break;
            }
        }
    }
}

void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
