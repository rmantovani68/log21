/*
* tcp server 
*/
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_MAX_LEN 256
#define PORT        8060

#define SA struct sockaddr
    
/* Function designed for chat between client and server. */
void func(int connfd)
{
    char buff[MSG_MAX_LEN];
    /* infinite loop for chat */
    for (;;) {
        bzero(buff, MSG_MAX_LEN);
    
        /* read the message from client and copy it in buffer */
        read(connfd, buff, sizeof(buff));
        /* print buffer which contains the client contents */
        printf("From client: %s\t To client : %s", buff, buff);
        /* and send that buffer to client */
        write(connfd, buff, sizeof(buff));
    
        /* if msg contains "Exit" then server exit and chat ended. */
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}
    
extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
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
    } else {
        printf("Socket successfully created..\n");
    }
    
    bzero(&servaddr, sizeof(servaddr));
    
    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(nPORT);
    
    /* Binding newly created socket to given IP and verification */
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
    }
    
    /* Now server is ready to listen and verification */
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    len = sizeof(cli);
    
    /* Accept the data packet from client and verification */
    connfd = accept(sockfd, (SA*)&cli, (socklen_t *)&len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
    
    /* Function for chatting between client and server */
    func(connfd);
    
    /* After chatting close the socket */
    close(sockfd);
}
