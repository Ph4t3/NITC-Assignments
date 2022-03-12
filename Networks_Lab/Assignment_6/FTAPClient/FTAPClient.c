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
        fprintf(graph_ptr, "%f\t%d\n", elapsed_time, speed / 1024);
        usleep(100000);
    }
}

void send_file(int sockfd) {

}

void recv_file(int sockfd)
{
    int recv_size, curr_seq_no = 0;
    FILE* fp;
    graph_ptr = fopen("stats.dat", "w");
    fp = fopen("outfile", "w");

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, timer_thread, NULL);

    while (1) {
        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        if (recv_size > 0 && packet->seq_no == curr_seq_no) {
            if (packet->size == 0) {
                pthread_cancel(timer_t);
                printf("%s\n", packet->data);
                return;
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
    packet = malloc(sizeof(Packet));

    while(1) {
        n = 0;
        printf("> ");
        memset(packet, 0, sizeof(Packet));
        while ((packet->data[n++] = getchar()) != '\n');
        packet->data[n - 1] = '\0';

        if(sockfd == -1) {
            if(strcmp("START", packet->data) == 0)
                sockfd = connect_socket();
            else
                printf("Please establish a connection first.\n");
        } else if (strcmp("QUIT", packet->data) == 0) {
            close(sockfd);
            sockfd = -1;
        } else {
            send(sockfd, packet, sizeof(Packet), 0);
            memset(packet, 0, sizeof(Packet));

            if(strncmp("StoreFile", packet->data, 9) == 0) {
                send_file(sockfd);
            } else if(strncmp("GetFile", packet->data, 7) == 0) {
                recv_file(sockfd);
            } else {
                recv(sockfd, packet, sizeof(Packet), 0);
                printf("%s\n", packet->data);
            }
        }
    }
}
