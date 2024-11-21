//
// Created by xiehao on 2024/11/20.
//
#include "../include/common.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>


// g++ -o client.o client-1.cpp

void sighandler_client(int sig) {
    if (SIGPIPE == sig) {
        INFO_PRINT("服务端已关闭\n");
    }
}


#define SERVER_PORT 8889

int main() {
    int status = 0;

    if (SIG_ERR != signal(SIGPIPE, sighandler_client)) {
        INFO_PRINT("注册信号处理成功: SIGSEGV\n");
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock_fd) {
        perror("socket fail: ");
        exit(-1);
    }

    // 初始化地址
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERVER_PORT);

    if (-1 == connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        perror("connect fail:  ");
    }

    string s;
    while (1) {
        cout << "你想对服务器说:";
        cin >> s;

        int writesize = write(sock_fd, s.c_str(), s.length());
        INFO_PRINT("writesize: %d\n", writesize);
        if (s.length() != writesize) {
            perror("write fail :");


            if (errno == EPIPE) {
                ERROR_PRINT("服务端已关闭\n");

                exit(-1);
            }

        }
    }

    close(sock_fd);
    return 0;

}
