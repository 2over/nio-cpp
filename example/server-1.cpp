//
// Created by xiehao on 2024/11/20.
//
#include "../include/common.h"

void sighandler(int sig) {
    if (SIGINT == sig) {
        INFO_PRINT("ctrl + c pressed\n");
        exit(-1);
    }
}

#define PORT 8889

void print_client_info(struct sockaddr_in *p) {
    int port = htons(p->sin_port);

    char ip[16];

    memset(ip, 0, sizeof(ip));

    inet_ntop(AF_INET, &(p->sin_addr.s_addr), ip, sizeof(ip));

    INFO_PRINT("client connected: %s(%d)\n", ip, port);
}

void do_service(int client_fd) {
    //写给client: hello
    string s = "hello\n";

    if (s.length() != write(client_fd, s.c_str(), s.length())) {
        perror("write fail: ");
        exit(-1);
    }

    char buff[1024] = {0};

    int readsize = 0;

    while (1) {
        memset(buff, 0, sizeof(buff));

        readsize = read(client_fd, buff, sizeof(buff));
        if (-1 == readsize) { // 出错了
            perror("read fail :");
            exit(-1);
        } else if (0 == readsize) { // 读完了
            printf("client quit\n");
            break;
        }

        printf("client say: %s\n", buff);

        if (!strcmp("bye", buff)) {
            break;
        }

    }
}

int main() {
    int status;

    if (SIG_ERR != signal(SIGSEGV, sighandler)) {
        INFO_PRINT("注册信号处理成功: SIGSEGV\n");
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == serv_sock) {
        ERROR_PRINT("socket fail\n");
        exit(-1);
    }

    // 端口复用
    int opt = 1;
    if (-1 == setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt fail:");
        exit(-1);
    }

    // 初始化socket元素
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // 绑定文件描述符和务器的ip和端口号
    status = bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (0 != status) {
        ERROR_PRINT("bind fail\n");

        perror("bind fail: ");

        exit(-2);
    }

    status = listen(serv_sock, 10);
    if (-1 == status) {
        ERROR_PRINT("listen fail\n");
        exit(-3);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));

    while (1) {
        INFO_PRINT("wait connect....\n");
        int client_fd = accept(serv_sock, (struct sockaddr *) &client_addr, &client_addr_len);

        print_client_info(&client_addr);

        do_service(client_fd);

        close(client_fd);
    }

    close(serv_sock);
}