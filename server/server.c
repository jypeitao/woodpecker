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

#define DEFAULT_PORT 8009
#define PENDING_MAX 8

#define BUFFER_SIZE 40
#define MAX_EVENTS 10


int start_server() {
    int epoll_fd;

    int remote_fd;
    struct sockaddr_in remote_addr;

    int server_socket_fd;
    struct sockaddr_in server_addr;

    socklen_t sockaddr_len = sizeof(struct sockaddr);


    if ((server_socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    if (bind(server_socket_fd, (struct sockaddr *) &server_addr, sockaddr_len) < 0) {
        perror("bind fail");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, PENDING_MAX) == -1) {
        perror("listen fail");
        exit(EXIT_FAILURE);
    }

    epoll_fd = epoll_create(MAX_EVENTS);
    if (epoll_fd == -1) {
        perror("epoll_create failed");
        exit(EXIT_FAILURE);
    }


    struct epoll_event events[MAX_EVENTS];

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_socket_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1) {
        perror("epoll_ctl:server_socket_fd register failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int num = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num == -1) {
            perror("start epoll_wait failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num; i++) {
            if (events[i].data.fd == server_socket_fd) {

                if ((remote_fd = accept(server_socket_fd, (struct sockaddr *) &remote_addr, &sockaddr_len)) < 0) {
                    perror("accept remote_fd failed");
                    exit(EXIT_FAILURE);
                }

                ev.events = EPOLLIN;
                ev.data.fd = remote_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, remote_fd, &ev) == -1) {
                    perror("epoll_ctl:epoll_fd register failed");
                    exit(EXIT_FAILURE);
                }
                printf("accept client %s/n", inet_ntoa(remote_addr.sin_addr));
            } else {
                char buf[BUFFER_SIZE];
                int len = recv(remote_fd, buf, BUFFER_SIZE, 0);
                if (len < 0) {
                    perror("receive from client failed");
                    exit(EXIT_FAILURE);
                }
                printf("receive from client:%s", buf);
                send(remote_fd, "I have received your message.", 30, 0);
            }
        }
    }

}
