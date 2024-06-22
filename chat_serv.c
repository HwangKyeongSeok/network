#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define NAME_SIZE 20

void* handle_clnt(void* arg);
void send_msg(char* msg, int len, int clnt_idx);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][NAME_SIZE];
pthread_mutex_t mutx;

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

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
        int str_len = read(clnt_sock, clnt_names[clnt_cnt], NAME_SIZE - 1);  // 클라이언트 이름 저장
        clnt_names[clnt_cnt][str_len] = '\0';  // null-terminate
        clnt_cnt++;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }

    close(serv_sock);
    return 0;
}

void* handle_clnt(void* arg) {
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];
    char name_msg[NAME_SIZE + BUF_SIZE + 3];  // 추가 공간을 위해 +3
    int clnt_idx;

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            clnt_idx = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutx);

    while ((str_len = read(clnt_sock, msg, sizeof(msg) - 1)) != 0) {
        msg[str_len] = '\0';
        // name_msg가 충분한 크기를 가지도록 확인
        snprintf(name_msg, sizeof(name_msg), "[%s] %s", clnt_names[clnt_idx], msg);
        name_msg[sizeof(name_msg) - 1] = '\0';  // 버퍼 오버플로 방지
        send_msg(name_msg, strlen(name_msg), clnt_idx);
    }

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
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

void send_msg(char* msg, int len, int clnt_idx) {  // send to all
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (i != clnt_idx) {
            write(clnt_socks[i], msg, len);
        }
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
