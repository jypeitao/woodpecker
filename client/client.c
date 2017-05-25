//
// Created by guotao on 5/25/17.
//

#include <netinet/in.h>
#include <stdio.h>
#include <memory.h>
#include <arpa/inet.h>

#define SERVER_IP "10.120.0.66"
#define SERVER_PORT 8009

int connect_server() {
    int socket_id;
    struct sockaddr_in sockaddr;

    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket creation failed");
        return -1;
    }

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &sockaddr.sin_addr.s_addr) <= 0) {
        perror("client socket_addr creation failed");
        return -1;
    }

    if (connect(socket_id, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
        perror("client connect server failed");
        return -1;
    }
    return socket_id;
}