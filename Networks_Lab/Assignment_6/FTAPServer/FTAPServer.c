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
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#define MAX 100
#define PORT 1235
#define BACKLOG 5
#define SIZE 500

typedef struct Packet {
    int size;
    int seq_no;
    int ack_no;
    char data[SIZE];
    int total_data;
} Packet;
Packet* packet;

typedef struct Users {
    char username[100];
    char password[100];
    struct Users *nextUser;
} Users;
Users *users;

void getUsers() {
    users = NULL;
    FILE *fp;
    fp = fopen("../logincred.txt", "r");

    char str[100];
    while (fgets(str, sizeof(str), fp)) {
        Users *user = malloc(sizeof(Users));
        strcpy(user->username, strtok(str, ", \n"));
        strcpy(user->password, strtok(NULL, ", \n"));
        user->nextUser = users;
        users = user;
    }
}

Users* findUser(char* username) {
    Users *head = users;
    while(head != NULL) {
        if(strcmp(username, head->username) == 0)
            return head;
        head = head->nextUser;
    }
    return NULL;
}

void recv_file(int sockfd, char* filename)
{
    int recv_size, curr_seq_no = 0;
    FILE* fp = fopen(filename, "w");

    while (1) {
        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        if (recv_size > 0 && packet->seq_no == curr_seq_no) {
            if (packet->size == 0)
                break;

            fwrite(packet->data, sizeof(char), packet->size, fp);
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

int send_file(int sockfd, char* filename) {
    int count, recv_size;
    FILE* fp = fopen(filename, "rb");
    int curr_seq_no = 0;

    if(fp == NULL)
        return 0;

    // Find the file size
    fseek(fp, 0L, SEEK_END);
    int total_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    while (1) {
        memset(packet, 0, sizeof(Packet));
        count = fread(packet->data, sizeof(char), SIZE, fp);
        packet->seq_no = curr_seq_no;
        packet->size = count;
        packet->total_data = total_file_size;

        if (count == 0)
            return 1;

        clock_t begin = clock();
        if (send(sockfd, packet, sizeof(*packet), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }

        memset(packet, 0, sizeof(Packet));
        recv_size = recv(sockfd, packet, sizeof(Packet), 0);

        clock_t end = clock();
        double RTT = (double)(end - begin) / CLOCKS_PER_SEC;
        if (recv_size == -1 || packet->ack_no != curr_seq_no) {
            perror("[-]Error: ACK Invalid");
            fseek(fp, -SIZE, SEEK_CUR);
        } else {
            printf("\rRound Trip Time (RTT) = %lfs", RTT);
            fflush(stdout);
            curr_seq_no++;
        }
    }
}

char* list_files() {
    DIR *dp;
    struct dirent *ep;
    char *out = calloc(0, 0);
    dp = opendir ("./");

    if (dp != NULL) {
        while ((ep = readdir (dp)) != NULL) {
            out = realloc(out, strlen(out) + strlen(ep->d_name) + 2);
            sprintf(out + strlen(out), "%s\n", ep->d_name);
        }

        out[strlen(out) - 1] = '\0';
        closedir (dp);
    } else
        perror ("Couldn't open the directory");

    return out;
}

// Main func for FTP protocol
void ftp(int connfd)
{
    int count, index, ret, authenticated = 0;
    Users* user = NULL;
    char cmd[20], arg[100], *temp;
    packet = malloc(sizeof(Packet));

    // infinite loop for chat
    while (1) {
        memset(packet, 0, sizeof(Packet));
        memset(cmd, 0, 20);
        memset(arg, 0, 100);

        if(!recv(connfd, packet, sizeof(Packet), 0)) // Read message from client
            return;

        strcpy(cmd,strtok(packet->data, " \n"));
        temp = strtok(NULL, " \n");
        if(temp != NULL)
            strcpy(arg, temp);

        memset(packet, 0, sizeof(Packet));
        printf("Cmd: %s, Argument: %s\n", cmd, arg);

        if(authenticated) {
            if (strcmp(cmd, "GetFile") == 0) {
                if(send_file(connfd, arg))
                    strcpy(packet->data, "File Transferred Successfully.");
                else
                    strcpy(packet->data, "File Not Found.");
            } else if (strcmp(cmd, "StoreFile") == 0) {
                recv_file(connfd, arg);
                strcpy(packet->data, "File Transferred Successfully.");
            } else if (strcmp(cmd, "CreateFile") == 0) {
                FILE *fp = fopen(arg, "w");
                fclose(fp);
                strcpy(packet->data, "File Created Successfully.");
            } else if (strcmp(cmd, "ListDir") == 0) {
                strcpy(packet->data, list_files());
            } else {
                strcpy(packet->data, "505: Command not supported");
            }
        } else {
            if (strcmp("USERN", cmd) == 0) {
                user = findUser(arg);

                if(user == NULL)
                    strcpy(packet->data, "301: Incorrect Username");
                else
                    strcpy(packet->data, "300: Correct Username; Need password");
            } else if(user != NULL && strcmp("PASSWD", cmd) == 0){
                if(strcmp(user->password, arg) == 0) {
                    authenticated = 1;
                    strcpy(packet->data, "305: User Authenticated with password");
                } else
                    strcpy(packet->data, "310: Incorrect password");
            } else {
                strcpy(packet->data, "505: Command not supported");
            }
        }
        send(connfd, packet, sizeof(Packet), 0);
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

    // Load the users file
    getUsers();

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
            ftp(connfd);
            close(connfd);
            exit(0);
        }

        close(connfd);
    }
}

