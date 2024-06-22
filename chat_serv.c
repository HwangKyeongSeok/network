#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256
#define NAME_SIZE 20
<<<<<<< HEAD

void* handle_clnt(void* arg);
void send_msg(char* msg, int len, int sock);
void send_private_msg(char* msg, int len, int sock, char* target_name);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE];
=======

typedef struct {
    int sock;
    char name[NAME_SIZE];
} Client;

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);
void send_private_msg(char* msg, int len, int sender_sock, char* sender_name);
void error_handling(char* msg);

int clnt_cnt = 0;
Client clnt_socks[MAX_CLNT];
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)
pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
<<<<<<< HEAD
        printf("Usage: %s <port>\n", argv[0]);
=======
        printf("Usage : %s <port>\n", argv[0]);
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
<<<<<<< HEAD
    if (serv_sock == -1)
        error_handling("socket() error");
=======
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
<<<<<<< HEAD

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt] = clnt_sock;
        read(clnt_sock, clnt_names[clnt_cnt], NAME_SIZE); // Receive client name
=======
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt].sock = clnt_sock;
        read(clnt_sock, clnt_socks[clnt_cnt].name, NAME_SIZE); // Å¬¶óÀÌ¾ğÆ® ÀÌ¸§ ÀĞ±â
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)
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
    int str_len = 0, i;
<<<<<<< HEAD
    char msg[BUF_SIZE];
    char target_name[NAME_SIZE];
    char* msg_start;

    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) {
        msg[str_len] = '\0'; // Ensure null-terminated string

        // Find the end of the sender's name
        msg_start = strchr(msg, ']');
        if (msg_start != NULL) {
            msg_start++; // Move past the ']'

            // Check if the message starts with '@' after the sender's name
            if (msg_start[0] == '@') {
                sscanf(msg_start, "@%s", target_name);
                // Find the end of the target name
                char* msg_content = strchr(msg_start, ' ');
                if (msg_content != NULL) {
                    send_private_msg(msg_content + 1, str_len - (msg_content - msg), clnt_sock, target_name);
                }
            }
            else {
                send_msg(msg, str_len, clnt_sock);
            }
        }
        else {
            send_msg(msg, str_len, clnt_sock);
        }
    }

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            while (i++ < clnt_cnt - 1) {
                clnt_socks[i] = clnt_socks[i + 1];
                strcpy(clnt_names[i], clnt_names[i + 1]);
            }
=======
    char msg[BUF_SIZE + NAME_SIZE + 2]; // ÀÌ¸§°ú ¸Ş½ÃÁö¸¦ ¸ğµÎ Æ÷ÇÔÇÒ ¼ö ÀÖ´Â ÃæºĞÇÑ Å©±â

    while ((str_len = read(clnt_sock, msg, sizeof(msg) - 1)) != 0)
    {
        msg[str_len] = 0; // ¹®ÀÚ¿­ Á¾·á ¹®ÀÚ Ãß°¡

        if (msg[NAME_SIZE] == '@') {
            send_private_msg(msg, str_len, clnt_sock, msg);
        }
        else {
            send_msg(msg, str_len);
        }
    }

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
    {
        if (clnt_sock == clnt_socks[i].sock)
        {
            while (i++ < clnt_cnt - 1)
                clnt_socks[i] = clnt_socks[i + 1];
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

<<<<<<< HEAD
void send_msg(char* msg, int len, int sock)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_socks[i] != sock) { // Exclude sender
            write(clnt_socks[i], msg, len);
        }
    }
    pthread_mutex_unlock(&mutx);
}

void send_private_msg(char* msg, int len, int sock, char* target_name)
{
    int i;
    int target_sock = -1;

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
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
        write(sock, error_msg, strlen(error_msg));
=======
void send_msg(char* msg, int len)   // send to all
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)
        write(clnt_socks[i].sock, msg, len);
    pthread_mutex_unlock(&mutx);
}

void send_private_msg(char* msg, int len, int sender_sock, char* sender_name)   // send to specific client
{
    int i;
    char* target_name = strtok(msg + NAME_SIZE + 1, " "); // Extract target name
    char* message = strtok(NULL, ""); // Extract message content
    char full_msg[BUF_SIZE + NAME_SIZE];

    if (message == NULL) {
        char error_msg[] = "Incorrect message format. Use @name message.\n";
        write(sender_sock, error_msg, sizeof(error_msg));
        return;
    }

    pthread_mutex_lock(&mutx);
    int target_found = 0;
    for (i = 0; i < clnt_cnt; i++) {
        if (strcmp(clnt_socks[i].name, target_name) == 0) {
            sprintf(full_msg, "[%s to %s] %s", sender_name, target_name, message);
            write(clnt_socks[i].sock, full_msg, strlen(full_msg));
            target_found = 1;
            break;
        }
    }
    if (!target_found) {
        char error_msg[] = "User not found.\n";
        write(sender_sock, error_msg, sizeof(error_msg));
>>>>>>> parent of 3b0759e (ì›ìƒíƒœ ë³µêµ¬)
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
