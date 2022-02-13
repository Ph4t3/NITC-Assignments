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
#define COUNT 100

struct {
    char* name;
    int count;
} fruits[] = {
    { "apple", COUNT },
    { "mango", COUNT },
    { "banana", COUNT },
    { "chikoo", COUNT },
    { "papaya", COUNT }
};

int fruit2index(char* name)
{
    for (int i = 0; i < sizeof(fruits) / sizeof(fruits[0]); i++) {
        if (!strcmp(name, fruits[i].name))
            return i;
    }
    return -1;
}

char* fruit2str()
{
    char* output = (char*)malloc(sizeof(char) * MAX);
    strcpy(output, "");

    for (int i = 0; i < sizeof(fruits) / sizeof(fruits[0]); i++) {
        char temp[MAX];
        sprintf(temp, "%s : %d\n", fruits[i].name, fruits[i].count);
        strcat(output, temp);
    }

    return output;
}

void decrementFruitCount(int connfd)
{
}

// Function designed for chat between client and server.
void fruitsHandler(int connfd)
{
    char buff[MAX], *fruit;
    int count, index, ret;

    // infinite loop for chat
    while (1) {
        bzero(buff, MAX);
        recv(connfd, buff, MAX, 0); // Read message from client
        printf("%s \n", buff);

        if (!strcmp(buff, "Fruits")) {
            strcpy(buff, "Enter the name of the fruit:");
            send(connfd, buff, strlen(buff), 0);

            bzero(buff, MAX);
            recv(connfd, buff, MAX, 0); // Read message from client
            fruit = strtok(buff, " ");
            sscanf(strtok(NULL, " "), "%d", &count);
            printf("%s %d\n", fruit, count);

            index = fruit2index(fruit);
            if (index == -1 || count > fruits[index].count) {
                bzero(buff, MAX);
                strcpy(buff, "Not Available\n");
                printf("%s\n", buff);
            } else {
                fruits[index].count -= count;
                bzero(buff, MAX);
                strcpy(buff, "Success\n");
                printf("%s\n", buff);
            }
        } else if (!strcmp(buff, "SendInventory")) {
            bzero(buff, MAX);
            strcpy(buff, fruit2str());
        } else if (strncmp("Exit", buff, 4) == 0) {
            printf("Client Exit...\n");
            break;
        } else {
            bzero(buff, MAX);
            strcpy(buff, "Not Available\n");
        }

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

        if (!fork()) {     // Child process
            close(sockfd); // child doesn't need the listener
            fruitsHandler(connfd);
            close(connfd);
            exit(0);
        }

        close(connfd);
    }
}
