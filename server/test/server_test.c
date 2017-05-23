#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 50

int main(int argc, char *argv[]) {

    if (argc <= 2) {
        printf("usage: %s ip_address port_number\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);


    struct sockaddr_in remote_addr; // 服务器端网络地址结构体     
    memset(&remote_addr, 0, sizeof(remote_addr)); // 数据初始化--清零
    remote_addr.sin_family = AF_INET; // 设置为IP通信
    inet_pton(AF_INET, ip, &remote_addr.sin_addr.s_addr);
    remote_addr.sin_port = htons(port); // 服务器端口号


    int client_sockfd;
    if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket creation failed");
        exit(EXIT_FAILURE);
    }
    // 将套接字绑定到服务器的网络地址上   
    if (connect(client_sockfd, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr)) < 0) {
        perror("connect to server failed");
        exit(EXIT_FAILURE);
    }

    char buf[BUFFER_SIZE + 1];  // 数据传送的缓冲区
    while (1) {
        printf("Please input the message:");
        scanf("%s", buf);
        if (strcmp(buf, "exit") == 0) {
            break;
        }

        int len = strlen(buf);
        printf("send to server++++++++++++++\n");
        printf("send to server %d Bytes : %s\n", len, buf);

        send(client_sockfd, buf, strlen(buf), 0);

        // 接收服务器端信息   
        buf[BUFFER_SIZE] = '\0';
        len = recv(client_sockfd, buf, BUFFER_SIZE, 0);

        printf("recv from server #######\n");

        int i = 0;
        for (i = 0; i < len; i++) {
            printf("recv buf all[%d] : %x[%c]\n", i, buf[i], buf[i]);
        }

        printf("receive  %d Bytes : %s\n", len, buf);

        if (len == BUFFER_SIZE) {
            printf("Maybe there is more... bug!!\n");
        }

        if (len < 0) {
            perror("receive from server failed");
            exit(EXIT_FAILURE);
        }
    }
    close(client_sockfd);// 关闭套接字     
    return 0;
}  
