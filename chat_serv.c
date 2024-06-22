#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256
#define NAME_SIZE 20

void* handle_clnt(void* arg);
void send_msg(char* msg, int len, int sender_sock);
void send_private_msg(char* msg, int len, char* target_name);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE];
pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt] = clnt_sock;
        read(clnt_sock, clnt_names[clnt_cnt], NAME_SIZE); // Receive client name
        clnt_cnt++;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

void* handle_clnt(void* arg)
{
    int clnt_sock = *((int*)arg);
    int str_len = 0;
    char msg[BUF_SIZE];
    char target_name[NAME_SIZE];
    char* msg_start;
    char* target_start;

    while ((str_len = read(clnt_sock, msg, sizeof(msg) - 1)) != 0) {
        msg[str_len] = '\0'; // Ensure null-terminated string

        // Find the end of the sender's name
        msg_start = strchr(msg, ']');
        if (msg_start != NULL) {
            msg_start++; // Move past the ']'

            // Check if the message starts with '@'
            if (msg_start[0] == ' ' && msg_start[1] == '@') {
                target_start = msg_start + 2; // Move past the '@'
                char* msg_content = strchr(target_start, ' ');
                if (msg_content != NULL) {
                    int name_len = msg_content - target_start;
                    strncpy(target_name, target_start, name_len);
                    target_name[name_len] = '\0'; // Null-terminate the target name

                    send_private_msg(msg_content + 1, strlen(msg_content + 1), target_name);
                }
                else {
                    send_msg(msg, str_len, clnt_sock); // Send as a normal message if no space found after @username
                }
            }
            else {
                send_msg(msg, str_len, clnt_sock); // Send as a normal message if no @username found
            }
        }
        else {
            send_msg(msg, str_len, clnt_sock); // Send as a normal message if no ] found
        }
    }

    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            while (i++ < clnt_cnt - 1) {
                clnt_socks[i] = clnt_socks[i + 1];
                strcpy(clnt_names[i], clnt_names[i + 1]);
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

void send_msg(char* msg, int len, int sender_sock)
{
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (clnt_socks[i] != sender_sock) { // Exclude sender
            write(clnt_socks[i], msg, len);
        }
    }
    pthread_mutex_unlock(&mutx);
}

void send_private_msg(char* msg, int len, char* target_name)
{
    int target_sock = -1;
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (strcmp(clnt_names[i], target_name) == 0) {
            target_sock = clnt_socks[i];
            break;
        }
    }

    if (target_sock != -1) {
        write(target_sock, msg, len);
    }
    else {
        char error_msg[BUF_SIZE] = "User not found\n";
        write(target_sock, error_msg, strlen(error_msg));
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
