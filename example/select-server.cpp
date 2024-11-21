//
// Created by xiehao on 2024/11/20.
//

#include "../include/common.h"
#include <sys/select.h>

void sighandler(int sig) {
    if (SIGINT == sig) {
        INFO_PRINT("ctrl+c pressed\n");
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

int main() {
    int status;
    int max_fd = 0;

    if (SIG_ERR != signal(SIGINT, sighandler)) {
        INFO_PRINT("注册信号处理成功: SIGINT\n");
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == serv_sock) {
        ERROR_PRINT("socket fail\n");
        exit(-1);
    }

    max_fd = serv_sock;

    // 端口复用
    int opt = 1;
    if (-1 == setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt fail: ");
        exit(-1);
    }

    // 初始化socket元素
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // 绑定文件描述符和服务器的ip和端口号
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

    fd_set fds;
    FD_ZERO(&fds);

    FD_SET(serv_sock, &fds);

    char buf[1024];

    while (1) {
        INFO_PRINT("select block....\n");

        int r = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (r < 0) {
            perror("select fail: ");
            break;
        }

        if (FD_ISSET(serv_sock, &fds)) {
            int client_fd = accept(serv_sock, (struct sockaddr *) &client_addr, &client_addr_len);
            max_fd = client_fd;

            print_client_info(&client_addr);

            INFO_PRINT("%d 加入fds\n", client_fd);

            FD_SET(client_fd, &fds);

            continue;
        }

        for (int i = serv_sock + 1; i < max_fd + 1; i++) {
            if (FD_ISSET(i, &fds)) {
                memset(buf, 0, sizeof(buf));

                int readsize = read(i, buf, sizeof(buf));

                if (0 == readsize) {
                    INFO_PRINT("客户端(%d) 退出\n", client_addr.sin_port);
                    close(i);
                    // 这里删除了fd,就得重新计算max_fd,但是无从计算
                    FD_CLR(i, &fds);
                } else if (readsize > 0) {
                    INFO_PRINT("客户端(%d): %s\n", client_addr.sin_port, buf);
                }
            }
        }
    }
    close(serv_sock);

}