#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX 100
#define PORT 1235
#define SIZE 500

typedef struct Packet {
    int size;
    int seq_no;
    int ack_no;
    char data[SIZE];
} Packet;

Packet* packet;
int curr_file_size = 0;

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
        printf("Connection Failed.\n");
        return -1;
    }

    printf("200: OK Connection is set up\n");
    return sockfd;
}

void* timer_thread()
{
    int prev_file_size = 0;
    float elapsed_time = 0;
    while (1) {
        int speed = (curr_file_size - prev_file_size) * 10; // In bytes per second
        prev_file_size = curr_file_size;
        elapsed_time += 0.1; // In Seconds
        fflush(stdout);
        printf("\rTransmission rate = %d KB/s ", speed / 1024);
        usleep(100000);
    }
}

int send_file(int sockfd, char* filename) {
    int count, recv_size;
    FILE* fp = fopen(filename, "rb");
    int curr_seq_no = 0;

    memset(packet, 0, sizeof(Packet));
    if(fp == NULL)
        return 0;

    pthread_t timer_t;
    curr_file_size = 0;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    while (1) {
        memset(packet, 0, sizeof(Packet));
        count = fread(packet->data, sizeof(char), SIZE, fp);
        packet->seq_no = curr_seq_no;
        packet->size = count;

        if (count == 0) {
            pthread_cancel(timer_t);
            return 1;
        }

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
            curr_file_size += count;
        }
    }
}

void recv_file(int sockfd, char* filename)
{
    int recv_size, curr_seq_no = 0;
    FILE* fp = fopen(filename, "w");

    pthread_t timer_t;
    curr_file_size = 0;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    while (1) {
        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        if (recv_size > 0 && packet->seq_no == curr_seq_no) {
            if (packet->size == 0) {
                pthread_cancel(timer_t);
                printf("\n%s\n", packet->data);
                break;
            }

            fwrite(packet->data, sizeof(char), packet->size, fp);
            curr_file_size += packet->size;
            curr_seq_no++;
        }

        memset(packet, 0, sizeof(Packet));
        packet->ack_no = curr_seq_no - 1;

        if (send(sockfd, packet, sizeof(*packet), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
    }

    fclose(fp);
}

int main()
{
    int sockfd = -1, n;
    char cmd[20], arg[100], *temp;
    packet = malloc(sizeof(Packet));

    while(1) {
        // Intialize Variables
        n = 0;
        memset(packet, 0, sizeof(Packet));
        memset(cmd, 0, 20);
        memset(arg, 0, 100);

        printf("> ");

        // Get Input
        while ((packet->data[n++] = getchar()) != '\n');
        packet->data[n - 1] = '\0';

        // Separate Command and Argument
        temp = malloc(sizeof(packet->data));
        strcpy(temp, packet->data);
        strcpy(cmd,strtok(temp, " \n"));
        temp = strtok(NULL, " \n");
        if(temp != NULL)
            strcpy(arg, temp);

        if(sockfd == -1) {
            if(strcmp("START", cmd) == 0)
                sockfd = connect_socket();
            else
                printf("Please establish a connection first.\n");
        } else if (strcmp("QUIT", cmd) == 0) {
            close(sockfd);
            sockfd = -1;
        } else {
            send(sockfd, packet, sizeof(Packet), 0);
            memset(packet, 0, sizeof(Packet));

            if(strcmp("StoreFile", cmd) == 0) {
                send_file(sockfd, arg);
                send(sockfd, packet, sizeof(Packet), 0);
                memset(packet, 0, sizeof(Packet));
                recv(sockfd, packet, sizeof(Packet), 0);
                printf("\n%s\n", packet->data);
            } else if(strcmp("GetFile", cmd) == 0) {
                recv_file(sockfd, arg);
            } else {
                recv(sockfd, packet, sizeof(Packet), 0);
                printf("%s\n", packet->data);
            }
        }
    }
}
