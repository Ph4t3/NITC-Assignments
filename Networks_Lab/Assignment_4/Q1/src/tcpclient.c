#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 256
#define PORT 8080

struct message {
    int client;
    char data[MAX];
};

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

    printf("Connected to the Server...\n");
    return sockfd;
}

void func(int sockfd)
{
    struct message* buff = malloc(sizeof(struct message));
    int n;
    struct pollfd pfds[2];

    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = 0;
    pfds[1].events = POLLIN;

    while (1) {
        printf("Enter Message: ");
        fflush(stdout);
        int poll_count = poll(pfds, 2, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        bzero(buff, sizeof(*buff));
        if (pfds[0].revents & POLLIN) {
            int nbytes = recv(pfds[0].fd, buff, sizeof *buff, 0);
            printf("\r[Client %d]: %s\n", buff->client, buff->data);
        } else {
            n = 0;
            while ((buff->data[n++] = getchar()) != '\n')
                ;
            buff->data[n - 1] = '\0';
            send(pfds[0].fd, buff->data, strlen(buff->data), 0);
        }
    }
}

int main()
{
    int sockfd;
    sockfd = connect_socket();
    func(sockfd);
    close(sockfd); // Close Socket
}
