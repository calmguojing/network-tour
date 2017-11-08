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

#define PORT 8888
#define MAXLINE 1024
#define MAXSTEN 20

int n = 0, client[MAXSTEN];
fd_set allset;

/*-------------------------------------------------*/
/*            list functions                       */
/*-------------------------------------------------*/

struct client_msg{
    char name[20];
    char *clientip;
    int fdnumber;
    int portnumber;
    struct client_msg *next;
};

struct client_msg *list_create(){
    struct client_msg *lhead;
    lhead = (struct client_msg *)malloc(sizeof(struct client_msg));
    lhead->next = NULL;

    return lhead;
}

void list_save(struct client_msg *head, char name[20], int ifdnumber, int iportnumber){
    struct client_msg *ihead, *inew;
    ihead = head;
    inew = (struct client_msg *)malloc(sizeof(struct client_msg));
    strcpy(inew->name, name);
    inew->fdnumber = ifdnumber;
    inew->portnumber = iportnumber;
    inew->next = NULL;

    if(NULL == head){
        ihead == inew;
    }else{
        while(ihead->next != NULL){
            ihead = ihead->next;
        }
        ihead->next = inew;
    }
}


void list_delete(struct client_msg *head, int dfd){
    struct client_msg *dhead;
    dhead = head;

    if(NULL == head){
        printf("list is empty!\n");
        return;
    }else if(NULL == head->next){
        if(head->fdnumber != dfd){
            printf("cannot find the socket fd!\n");
            return;
        }else{
            dhead = dhead->next;
        }
    }else{
        while(dhead->next->fdnumber != dfd){
            dhead = dhead->next;
        }
        dhead->next = dhead->next->next;
    }
}


char *list_find(struct client_msg *head, int ffd){
    struct client_msg *fhead;
    fhead = head;

    if(NULL == head){
        printf("list is empty!\n");
        return NULL;
    }else{
        while(fhead->fdnumber != ffd){
            fhead = fhead->next;
        }
        return fhead->name;
    }
}

void list_print(struct client_msg *head){
    struct client_msg *phead;
    phead = head;

    if(NULL == head){
        printf("list is empty!\n");
    }else{
        printf("\n\nthe current online total number: %d\n", n);
        printf("\n-------------------------------------------------------------------------\n");
        printf("\t\tnickname: \t\tportnumber: \t\t socket fd: \n");
        while(phead != NULL){
            printf("\t\t%s\t\t%d\t\t%d\n",
                    phead->name, phead->portnumber, phead->fdnumber);
            phead = phead->next;
        }
        printf("\n-------------------------------------------------------------------------\n");
    }
}


/*-------------------------------------------------------*/
/*                                                       */
/*                server functions                       */
/*-------------------------------------------------------*/
int server_create(){
    int listenfd;
    struct sockaddr_in servaddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket error:%s\n",strerror(errno));
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        printf("bind error:%s\n", strerror(errno));
        exit(1);
    }

    if(listen(listenfd, MAXSTEN) == -1){
        printf("listen error:%s\n", strerror(errno));
        exit(1);
    }

    printf("waiting client to connect... ...\n");
    return listenfd;
}

int server_accept(int listenfd, struct client_msg *head){
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    char tempname[20];

    printf("new client is trying to connect the server... ...\n");
    clilen = sizeof(cliaddr);
    if((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) == -1){
        printf("accept error :%s\n", strerror(errno));
        exit(1);
    }
    recv(connfd, tempname, sizeof(tempname), 0);
    printf("client connection success! obtaining client's information... ...\n");
    printf("nickname: %s, portnumber: %d, IP: %s\n", 
            tempname, ntohs(cliaddr.sin_port), inet_ntoa(cliaddr.sin_addr));
    list_save(head, tempname, connfd, ntohs(cliaddr.sin_port));
    n++;
    list_print(head);
    return connfd;
}

void server_recv(struct client_msg *head, int sockfd, int i){
    char name[20], buff[MAXLINE + 1];
    int len, k;
    memset(buff, 0, sizeof(buff));
    len = recv(sockfd, buff, sizeof(buff), 0);

    if(len == 0){
        strcpy(name, list_find(head, sockfd));
        printf("\nuser%s quit the chatroom!\n", name);
        close(sockfd);
        FD_CLR(sockfd, &allset);
        list_delete(head, sockfd);
        client[i] = -1;
        n--;
        list_print(head);
    }else if(len < 0){
        strcpy(name, list_find(head, sockfd));
        printf("\nnetwork error! user %s offline!\n", name);
        printf("error:%s\n", strerror(errno));
        close(sockfd);
        FD_CLR(sockfd, &allset);
        list_delete(head, sockfd);
        client[i] = -1;
        n--;
        list_print(head);
    }else{
        printf("%s",buff);
        for(k=0; k<MAXSTEN; k++){
            if(client[i] > 0){
                send(client[k], buff, sizeof(buff), 0);
            }
        }
        memset(buff, 0, sizeof(buff));
    }
}

void server_command(struct client_msg *head){
    int chance, fd, j;
    char commandbuff[20];
    fgets(commandbuff, sizeof(commandbuff), stdin);

    if(strcmp(commandbuff, "command\n") == 0){
        printf("\n\n\n1.list the server member\n2.disconnect one client\n3.exit the server\n");
        printf("please input: ");
        scanf("%d", &chance);

        switch(chance){
            case 1:
                list_print(head);
                break;
            case 2:
                printf("please input the fd number of the client: ");
                scanf("%d", &fd);
                memset(commandbuff, 0, sizeof(commandbuff));
                strcpy(commandbuff, list_find(head, fd));
                list_delete(head, fd);
                close(fd);
                FD_CLR(fd, &allset);
                for(j=0; j<MAXSTEN; j++){
                    if(client[j] == fd){
                        client[j] = -1;
                        break;
                    }
                }
                n--;
                printf("\nuser%s forced to offline!\n",commandbuff);
                list_print(head);
                break;
            case 3:
                printf("server has already exit!\n");
                exit(0);
                break;
            default:
                printf("invalid command!\n");
                break;
        }
    }
}

int main(){
    int listenfd, sockfd;
    int i, maxfd;
    struct client_msg *head;

    listenfd = server_create();
    maxfd = listenfd;

    for(i=0; i<MAXSTEN; i++){
        client[i] = -1;
    }

    head = list_create();

    while(1){
        FD_ZERO(&allset);
        FD_SET(0, &allset);
        FD_SET(listenfd, &allset);

        for(i=0; i<MAXSTEN; i++){
            if(client[i] > 0){
                FD_SET(client[i], &allset);
            }
        }

        select(maxfd+1, &allset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &allset)){
            for(i=0; i<MAXSTEN; i++){
                if(client[i] < 0){
                    client[i] = server_accept(listenfd, head);
                    FD_SET(client[i], &allset);
                    break;
                }
            }
            if(i == MAXSTEN){
                printf("up to connection limit! cannot create new connection!\n");
            }
            if(client[i] > maxfd){
                maxfd = client[i];
            }
        }
        if(FD_ISSET(0, &allset)){
            server_command(head);
        }
        for(i=0; i<MAXSTEN; i++){
            if((sockfd = client[i]) < 0){
                continue;
            }else{
                if(FD_ISSET(sockfd, &allset)){
                    server_recv(head, sockfd, i);
                    break;
                }
            }
        }
    }
    return 0;
}
