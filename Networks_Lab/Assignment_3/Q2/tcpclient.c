#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 80
#define PORT 8080

int connect_socket()
{
    int sockfd, connfd, ret;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("ERR: Socket creation failed.\n");
        exit(0);
    }

    // assign IP, PORT
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret != 0) {
        printf("ERR: Server connection failed.\n");
        exit(0);
    }

    return sockfd;
}

void func(int sockfd)
{
    char buff[MAX];
    int n, choice;
    while (1) {
        bzero(buff, sizeof(buff));
        printf("1. Fruits\n");
        printf("2. SendInventory\n");
        printf("3. Exit\n");
        printf("Enter Choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            strcpy(buff, "Fruits");
            send(sockfd, buff, strlen(buff), 0);

            bzero(buff, sizeof(buff));
            recv(sockfd, buff, sizeof(buff), 0);
            printf("%s", buff);

            n = 0;
            bzero(buff, sizeof(buff));
            scanf(" ");
            while ((buff[n++] = getchar()) != '\n')
                ;
            send(sockfd, buff, strlen(buff), 0);
        } else if (choice == 2) {
            strcpy(buff, "SendInventory");
            send(sockfd, buff, strlen(buff), 0);
        } else {
            strcpy(buff, "Exit");
            send(sockfd, buff, strlen(buff), 0);
            break;
        }

        bzero(buff, sizeof(buff));
        recv(sockfd, buff, sizeof(buff), 0);
        printf("%s\n", buff);
    }
}

int main()
{
    int sockfd;
    sockfd = connect_socket();
    func(sockfd);
    close(sockfd); // Close Socket
}
