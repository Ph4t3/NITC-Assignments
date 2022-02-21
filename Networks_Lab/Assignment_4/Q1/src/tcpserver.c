#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 256
#define PORT 8080
#define BACKLOG 5

struct message {
    int client;
    char data[MAX];
};

int init_socket()
{
    int listener, ret;
    struct sockaddr_in serv_addr;

    // Create Socket
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        printf("ERR: Socket creation failed.\n");
        exit(0);
    }

    // Assign IP, PORT
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // Bind Socket
    ret = bind(listener, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret != 0) {
        printf("ERR: Socket bind failed.\n");
        exit(0);
    }

    // Listen for connection
    if ((listen(listener, BACKLOG)) != 0) {
        printf("ERR: Listen failed.\n");
        exit(0);
    }

    return listener;
}

void add_pfd(struct pollfd* pfds[], int newfd, int* pfd_count)
{
    (*pfd_count)++;
    *pfds = realloc(*pfds, sizeof(struct pollfd) * (*pfd_count));
    (*pfds)[*pfd_count - 1].fd = newfd;
    (*pfds)[*pfd_count - 1].events = POLLIN;
}

void del_pfd(struct pollfd* pfds[], int idx, int* pfd_count)
{
    (*pfd_count)--;
    (*pfds)[idx] = (*pfds)[(*pfd_count)];
    *pfds = realloc(*pfds, sizeof(struct pollfd) * (*pfd_count));
}

// Driver function
int main()
{
    int connfd, listener, pfd_count = 0;
    struct sockaddr_in their_addr;
    struct pollfd* pfds = malloc(sizeof *pfds);
    struct message* buff = malloc(sizeof(struct message));
    socklen_t addr_size;

    // Initialize Socket
    listener = init_socket();
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;
    pfd_count = 1;

    while (1) {
        int poll_count = poll(pfds, pfd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < pfd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == listener) {
                    // Accept the data packet from client and verification
                    addr_size = sizeof(their_addr);
                    connfd = accept(listener, (struct sockaddr*)&their_addr, &addr_size);
                    if (connfd < 0) {
                        printf("ERR: Server accept failed.\n");
                        exit(0);
                    }

                    printf("Client %d connected.\n", pfd_count);
                    add_pfd(&pfds, connfd, &pfd_count);
                } else {
                    bzero(buff, MAX);
                    int nbytes = recv(pfds[i].fd, buff->data, MAX, 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("Client %d left.\n", i);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd);
                        del_pfd(&pfds, i, &pfd_count);
                    } else {
                        printf("[Client %d]: %s\n", i, buff->data);
                        buff->client = i;
                        for (int j = 0; j < pfd_count; j++) {
                            if (pfds[j].fd != listener && i != j) {
                                if (send(pfds[j].fd, buff, sizeof *buff, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
