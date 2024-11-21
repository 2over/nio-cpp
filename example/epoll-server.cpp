//
// Created by xiehao on 2024/11/21.
//
#include "../include/common.h"
#include <sys/epoll.h>
#include <fcntl.h>

void sighandler(int sig) {
    if (SIGINT == sig) {
        INFO_PRINT("ctrl+c pressed\n");
        exit(-1);
    }
}

#define PORT 8889

void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);

    if (opts < 0) {
        perror("fcntl(sock, GETFL)");
        exit(1);
    }


    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts");
        exit(1);
    }

}

void print_client_info(struct sockaddr_in *p) {
    int port = htons(p->sin_port);

    char ip[16];

    memset(ip, 0, sizeof(ip));
    inet_ntop(AF_INET, &(p->sin_addr.s_addr), ip, sizeof(ip));

    INFO_PRINT("client connected : %s(%d)\n", ip, port);
}

int main() {
    int status;

    if (SIG_ERR != signal(SIGINT, sighandler)) {
        INFO_PRINT("注册信号处理成功: SIGINT\n");
    }


    // 创建epoll
    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("epoll_create fail ");
        exit(1);
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == serv_sock) {
        ERROR_PRINT("socket fail\n");
        exit(-1);
    }

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

    // ==========
    struct epoll_event event;
    struct epoll_event resevent[10];

    event.events = EPOLLIN | EPOLLET;

    event.data.fd = serv_sock;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_sock, &event);

    char buf[1024] = {0};

    for (;;) {
        int read_num = epoll_wait(epoll_fd, resevent, 10, 0);
        for (int i = 0; i < read_num; i++) {
            if (serv_sock == resevent[i].data.fd) {
                int client_fd = accept(serv_sock, (struct sockaddr *) &client_addr, &client_addr_len);
                if (client_fd < 0) {
                    perror("accept fail: ");
                    exit(-1);
                }

                print_client_info(&client_addr);

                event.data.fd = client_fd;
                event.events = EPOLLIN | EPOLLET;

                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            } else if (resevent[i].events & EPOLLIN) {
                memset(buf, 0, sizeof(buf));

                int readsize = read(resevent[i].data.fd, buf, sizeof(buf));
                if (0 == readsize) {
                    INFO_PRINT("client(%d) quit...\n", client_addr.sin_port);

                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, resevent[i].data.fd, &event);

                    close(resevent[i].data.fd);
                } else if (readsize > 0) {
                    INFO_PRINT("client say: %s\n", buf);
                    continue;
                }
            } else if (resevent[i].events & EPOLLOUT) {
                INFO_PRINT("发生写事件\n");
            } else {
                ERROR_PRINT("未知事件\n");
            }
        }
    }

    close(serv_sock);
    close(epoll_fd);
}
