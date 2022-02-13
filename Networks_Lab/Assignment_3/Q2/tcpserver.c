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
#define PORT 1235
#define BACKLOG 5
#define SIZE 500

typedef struct Packet {
    int size;
    int seq_no;
    int ack_no;
    char data[SIZE];
} Packet;
Packet* packet;

char* randstring(size_t sizegth)
{
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    char* randomString = NULL;

    if (sizegth) {
        randomString = malloc(sizeof(char) * (sizegth + 1));

        if (randomString) {
            for (int n = 0; n < sizegth; n++) {
                int key = rand() % (int)(sizeof(charset) - 1);
                randomString[n] = charset[key];
            }
            randomString[sizegth] = '\0';
        }
    }

    return randomString;
}

void send_file(int sockfd)
{
    int count, recv_size;
    FILE* fp = fopen("myfile.bin", "rb");
    int curr_seq_no = 0;

    while (1) {
        memset(packet, 0, sizeof(Packet));
        count = fread(packet->data, sizeof(char), SIZE, fp);
        packet->seq_no = curr_seq_no;
        packet->size = count;

        if (send(sockfd, packet, sizeof(*packet), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }

        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        if (recv_size == -1 || packet->ack_no != curr_seq_no) {
            perror("[-]Error: ACK Invalid");
            fseek(fp, -SIZE, SEEK_CUR);
        } else {
            curr_seq_no++;
        }

        if (count == 0)
            return;
    }
}

// Function designed for chat between client and server.
void chatHandler(int connfd)
{
    int count, index, ret;
    packet = malloc(sizeof(Packet));

    // infinite loop for chat
    while (1) {
        memset(packet, 0, sizeof(Packet));
        recv(connfd, packet, sizeof(Packet), 0); // Read message from client
        printf("%s \n", packet->data);

        if (!strcmp(packet->data, "GivemeyourVideo")) {
            send_file(connfd);
            /* strcpy(buff, "End Of File"); */
            /* printf("File sent successfully...\n"); */
        } else if (strcmp(packet->data, "Bye") == 0) {
            printf("Client Exit...\n");
            break;
        } else {
            memset(packet, 0, sizeof(Packet));
            strcpy(packet->data, randstring(20));
            send(connfd, packet, sizeof(Packet), 0);
        }
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

    addr_size = sizeof(their_addr);
    connfd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
    if (connfd < 0) {
        printf("ERR: Server accept failed.\n");
        exit(0);
    }

    chatHandler(connfd);
    close(connfd);
}
