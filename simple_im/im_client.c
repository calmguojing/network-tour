#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>

#define PORT 8888
#define MAXLINE 1024

int main(int argc, char *argv[]){
    int sockfd, maxfd, len;
    char msg1[MAXLINE + 1];
    char msg2[MAXLINE + 1];
    char msg3[MAXLINE + 1];
    char name[40], *str;
    char strquit[] = {"quit\n"};
    int quit = 1;

    static char timestr[40];
    struct sockaddr_in servaddr;
    struct hostent *host;

    time_t t;
    struct tm *nowtime;
    fd_set allset;

    if(argc != 2){ 
        fprintf(stderr,"Usage:%s hostname \a\n",argv[0]); 
        exit(1); 
    } 

    if((host = gethostbyname(argv[1])) == NULL){ 
        fprintf(stderr,"Gethostname error\n"); 
        exit(1); 
    } 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket error: %d %s\n", errno, strerror(errno));
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = *((struct in_addr *)host->h_addr);

    printf("connecting the server... ...\n");
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        printf("connect error: %s\n", strerror(errno));
        exit(1);
    }
    printf("connect the server successfully!\n");
    printf("please input your nickname: ");
    scanf("%s", name);
    send(sockfd, name, sizeof(name), 0);
    printf("%s welcome to the chatroom!(quit->quit)\n", name);
    fflush(stdin);
    maxfd = sockfd;

    while(1){
        FD_ZERO(&allset);
        FD_SET(sockfd, &allset);
        FD_SET(0, &allset);
        memset(msg1, 0, sizeof(msg1));
        memset(msg2, 0, sizeof(msg2));
        memset(msg3, 0, sizeof(msg3));

        select(maxfd+1, &allset, NULL, NULL, NULL);
        if(FD_ISSET(0, &allset)){
            fgets(msg1, sizeof(msg1), stdin);
            if(strcmp(msg1, strquit) == 0){
                close(sockfd);
                printf("lose the connection with server, quit the chatroom!\n");
                quit = 0;
            }else{
                time(&t);
                nowtime = localtime(&t);
                strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", nowtime);
                snprintf(msg2, sizeof(msg2), "%s %s:%s",timestr, name, msg1);

                send(sockfd, msg2, sizeof(msg2), 0);
            }
        }
        if(FD_ISSET(sockfd, &allset)){
            if(quit){
                len = recv(sockfd, msg3, sizeof(msg3), 0);
                if(len <= 0){
                    printf("server error, lose the connection with server!\n");
                    quit = 0;
                    close(sockfd);
                }else{
                    printf("%s", msg3);
                }
            }else{
                printf("lose the connection with server, cannot send the message!\n");
            }
        }

    }

    return 0;
}

