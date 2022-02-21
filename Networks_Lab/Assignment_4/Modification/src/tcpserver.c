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
#define PORT 8081
#define BACKLOG 5

char* password = "randompass";
struct message* buff;

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

int authenticate(int connfd, int pfd_count)
{
    bzero(buff, MAX);
    int nbytes = recv(connfd, buff->data, MAX, 0);

    if (strcmp(buff->data, password) == 0) {
        bzero(buff, MAX);
        buff->client = 0;
        sprintf(buff->data, "%d Welcome to project COBRA", pfd_count);
        if (send(connfd, buff, sizeof *buff, 0) == -1) {
            perror("send");
        }
        return 1;
    }

    bzero(buff, MAX);
    buff->client = 0;
    strcpy(buff->data, "0 Authentication Failed");
    if (send(connfd, buff, sizeof *buff, 0) == -1) {
        perror("send");
    }

    return 0;
}

// Driver function
int main()
{
    int connfd, listener, pfd_count = 0;
    struct sockaddr_in their_addr;
    struct pollfd* pfds = malloc(sizeof *pfds);
    buff = malloc(sizeof(struct message));
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

                    int ret = authenticate(connfd, pfd_count);
                    if (ret) {
                        printf("Co-PI %d connected.\n", pfd_count);
                        add_pfd(&pfds, connfd, &pfd_count);
                    } else {
                        printf("Co-PI %d Verification failed.\n", pfd_count);
                        close(connfd);
                    }
                } else {
                    bzero(buff, MAX);
                    int nbytes = recv(pfds[i].fd, buff->data, MAX, 0);
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("Co-PI %d left.\n", i);
                        } else {
                            perror("recv");
                        }

                        bzero(buff, MAX);
                        buff->client = i;
                        strcpy(buff->data, "Bye");
                        for (int j = 0; j < pfd_count; j++) {
                            if (pfds[j].fd != listener && i != j) {
                                if (send(pfds[j].fd, buff, sizeof *buff, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }

                        close(pfds[i].fd);
                        del_pfd(&pfds, i, &pfd_count);
                    } else {
                        printf("[Co-PI %d]: %s\n", i, buff->data);
                        buff->client = i;
                        for (int j = 0; j < pfd_count; j++) {
                            if (pfds[j].fd != listener && i != j) {
                                if (send(pfds[j].fd, buff, sizeof *buff, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }

                        if (strcmp(buff->data, "Bye") == 0) {
                            close(pfds[i].fd);
                            del_pfd(&pfds, i, &pfd_count);
                        }
                    }
                }
            }
        }
    }
}
