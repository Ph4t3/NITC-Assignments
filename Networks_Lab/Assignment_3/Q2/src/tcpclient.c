#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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
FILE* graph_ptr;

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
        fprintf(graph_ptr, "%f\t%d\n", elapsed_time, speed / 1024);
        usleep(100000);
    }
}

void write_file(int sockfd)
{
    int recv_size, curr_seq_no = 0, flag = 0;
    FILE* fp;
    graph_ptr = fopen("stats.dat", "w");
    fp = fopen("outfile", "wb");

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    while (!flag) {
        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        if (recv_size > 0 && packet->seq_no == curr_seq_no) {
            /* printf("Recv: %d\nLen: %d\n", recv_size, packet->len); */
            fwrite(packet->data, sizeof(char), packet->size, fp);
            curr_file_size += packet->size;
            curr_seq_no++;

            if (packet->size == 0) {
                pthread_cancel(timer_t);
                flag = 1;
            }
        }

        memset(packet, 0, sizeof(Packet));
        packet->ack_no = curr_seq_no - 1;

        if (send(sockfd, packet, sizeof(*packet), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
    }
}

void func(int sockfd)
{
    int n, choice;
    packet = malloc(sizeof(Packet));
    while (1) {
        memset(packet, 0, sizeof(Packet));
        n = 0;
        while ((packet->data[n++] = getchar()) != '\n')
            ;
        packet->data[n - 1] = '\0';
        send(sockfd, packet, sizeof(Packet), 0);

        if (strcmp("Bye", packet->data) == 0) {
            printf("Client Exit...\n");
            break;
        } else if (strcmp("GivemeyourVideo", packet->data) == 0) {
            write_file(sockfd);
            printf("\nFile received successfully...\n");
        } else {
            memset(packet, 0, sizeof(Packet));
            recv(sockfd, packet, sizeof(Packet), 0);
            printf("Server: %s\n", packet->data);
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
