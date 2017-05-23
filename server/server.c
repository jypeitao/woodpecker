//
// Created by peter on 5/18/17.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <asm/errno.h>
#include <errno.h>

#define DEFAULT_PORT 8009
#define PENDING_MAX 5

#define BUFFER_SIZE 8
#define MAX_EVENT_NUMBER 10

#define SUCCESS (0)

#define FAILURE (-1)


int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epoll_fd, int fd, bool enable_et) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (enable_et) {
        ev.events |= EPOLLET;
    }

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("function addfd:EPOLL_CTL_ADD failed");
        exit(EXIT_FAILURE);
    }
}

void removefd(int epoll_fd, int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        perror("function addfd:EPOLL_CTL_DEL failed");
        exit(EXIT_FAILURE);
    }
}

void level_triggered_deal(struct epoll_event *events, int num, int epoll_fd, int listen_fd) {
    char buf[BUFFER_SIZE];
    int i = 0;
    for (i = 0; i < num; i++) {
        int socketfd = events[i].data.fd;
        if (socketfd == listen_fd) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            int client_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);

            addfd(epoll_fd, client_fd, false);
            setnonblocking(client_fd);

            printf("accept client %s\n", inet_ntoa(client_addr.sin_addr));
        } else if (events[i].events & EPOLLIN) {
            printf("event trigger once \n");
            memset(buf, '\0', BUFFER_SIZE);
            ssize_t len = recv(socketfd, buf, BUFFER_SIZE - 1, 0);
            printf("len =  %ld\n", len);
            if (len <= 0) {
                removefd(epoll_fd, socketfd);
                close(socketfd);
                continue;
            }
            printf("receive from client:%s\n", buf);
            send(socketfd, "I have received your message.", 30, 0);
        } else {
            printf("Some unexpected happened!\n");
        }
    }
}


void edge_triggered_deal(struct epoll_event *events, int num, int epoll_fd, int listen_fd) {
    char buf[BUFFER_SIZE];
    int i = 0;
    for (i = 0; i < num; i++) {
        int socketfd = events[i].data.fd;
        if (socketfd == listen_fd) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);

            int client_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);

            addfd(epoll_fd, client_fd, true);

            printf("accept client %s\n", inet_ntoa(client_addr.sin_addr));
        } else if (events[i].events & EPOLLIN) {
            printf("event trigger once \n");
            while (1) {
                memset(buf, '\0', BUFFER_SIZE);
                ssize_t len = recv(socketfd, buf, BUFFER_SIZE - 1, 0);
                printf("len =  %ld\n", len);
                if (len < 0) {
                    if ((EAGAIN == errno) || (EWOULDBLOCK == errno)) {
                        printf("read later\n");
                        break;
                    }
                    removefd(epoll_fd, socketfd);
                    close(socketfd);
                    break;
                } else if (0 == len) {
                    removefd(epoll_fd, socketfd);
                    close(socketfd);
                    break;
                } else {
                    printf("receive from client:%s\n", buf);
                }
                send(socketfd, "I have received your message.", 30, 0);
            }
        } else {
            printf("Some unexpected happened!\n");
        }
    }
}

int start_server() {
    int epoll_fd;

    int server_socket_fd;
    struct sockaddr_in server_addr;

    if ((server_socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return FAILURE;
    }

    int reuse = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    if (bind(server_socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind fail");
        return FAILURE;
    }

    if (listen(server_socket_fd, PENDING_MAX) == -1) {
        perror("listen fail");
        return FAILURE;
    }

    epoll_fd = epoll_create(MAX_EVENT_NUMBER);
    if (epoll_fd == -1) {
        perror("epoll_create failed");
        return FAILURE;
    }

    addfd(epoll_fd, server_socket_fd, true);
    setnonblocking(server_socket_fd);

    struct epoll_event events[MAX_EVENT_NUMBER];
    while (1) {
        int num = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (num == -1) {
            perror("start epoll_wait failed");
            return FAILURE;
        }
        printf("++++++++++num = %d\n", num);
        //edge_triggered_deal(events, num, epoll_fd, server_socket_fd);
        level_triggered_deal(events, num, epoll_fd, server_socket_fd);
    }

    close(server_socket_fd);
    return SUCCESS;


}
