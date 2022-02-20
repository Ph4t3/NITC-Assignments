#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 100
#define PORT 8080
#define BACKLOG 5

int client = 1;
char* buff;

void evaluate(char* buff)
{
    float a, b;
    int ret;
    char op;

    ret = sscanf(buff, "%f %c %f", &a, &op, &b);
    bzero(buff, MAX);
    if (ret != 3) {
        strcpy(buff, "Invalid Expression");
        return;
    }

    switch (op) {
    case '+':
        sprintf(buff, "%f", a + b);
        break;
    case '-':
        sprintf(buff, "%f", a - b);
        break;
    case '*':
        sprintf(buff, "%f", a * b);
        break;
    case '/':
        if (b == 0) {
            sprintf(buff, "Division with 0 not permitted");
            break;
        }
        sprintf(buff, "%f", a / b);
        break;
    default:
        strcpy(buff, "Invalid Expression 2");
    }
    return;
}

// Function designed for chat between client and server.
void calculator(int connfd)
{
    buff = malloc(sizeof(char) * MAX);

    // infinite loop for chat
    while (1) {
        bzero(buff, MAX);
        recv(connfd, buff, MAX, 0); // Read message from client
        printf("[Client %d]: %s \n", client, buff);

        if (strncmp("Exit", buff, 4) == 0) {
            printf("\nClient %d Exit...\n\n", client);
            break;
        }

        evaluate(buff);
        printf("[Server]: %s \n", buff);
        send(connfd, buff, strlen(buff), 0);
    }
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

// Function to reap zombie processes
void reaper()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

int init_socket()
{
    int sockfd, ret;
    struct sockaddr_in serv_addr;

    // Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("ERR: Socket creation failed.\n");
        exit(0);
    }

    // Assign IP, PORT
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // Bind Socket
    ret = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret != 0) {
        printf("ERR: Socket bind failed.\n");
        exit(0);
    }

    // Listen for connection
    if ((listen(sockfd, BACKLOG)) != 0) {
        printf("ERR: Listen failed.\n");
        exit(0);
    }

    return sockfd;
}

// Driver function
int main()
{
    int connfd, sockfd;
    struct sockaddr_in their_addr;
    socklen_t addr_size;

    // Initialize Socket
    sockfd = init_socket();

    // Reap all dead processes
    reaper();

    while (1) {
        // Accept the data packet from client and verification
        addr_size = sizeof(their_addr);
        connfd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
        if (connfd < 0) {
            printf("ERR: Server accept failed.\n");
            exit(0);
        }

        if (!fork()) { // Child process
            printf("\nConnected with Client %d\n\n", client);
            close(sockfd); // child doesn't need the listener
            calculator(connfd);
            close(connfd);
            exit(0);
        }

        client++;
        close(connfd);
    }
}
