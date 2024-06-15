/*
* tcp client
*/
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define MSG_MAX_LEN 256
#define PORT        8080

#define SA struct sockaddr

void func(int sockfd)
{
    char buff[MSG_MAX_LEN];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    int nPORT = PORT, nOpt, nErr=0;

    while ((nOpt = getopt(argc, argv, "p:")) != -1) {
        switch(nOpt){
            case 'p':    /* port */
                nPORT=atoi(optarg);
            break;
            default:
                nErr++;
            break;
        }
    }
    if(nPORT==0){
        fprintf(stderr, "port not specified - using %d\n", nPORT);
    }




    /* socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(nPORT);

    /* connect the client socket to server socket */
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    /* function for chat */
    func(sockfd);

    /* close the socket */
    close(sockfd);
}
