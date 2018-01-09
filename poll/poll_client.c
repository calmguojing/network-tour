#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#define portnumber 3333
#define MAXLINE     1024
static void handle_connection(int sockfd);
int main(int argc, char *argv[]) 
{ 
	int sockfd; 
	char buffer[1024]; 
	struct sockaddr_in server_addr; 
	struct hostent *host; 

        /* ʹ��hostname��ѯhost ���� */
	if(argc!=2) 
	{ 
		fprintf(stderr,"Usage:%s hostname \a\n",argv[0]); 
		exit(1); 
	} 

	if((host=gethostbyname(argv[1]))==NULL) 
	{ 
		fprintf(stderr,"Gethostname error\n"); 
		exit(1); 
	} 

	/* �ͻ�����ʼ���� sockfd������ */ 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 

	/* �ͻ�����������˵����� */ 
	bzero(&server_addr,sizeof(server_addr)); // ��ʼ��,��0
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  // (���������ϵ�short����ת��Ϊ�����ϵ�short����)�˿ں�
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP��ַ
	
	/* �ͻ��������������� */ 
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 
	handle_connection(sockfd);

	/* ����ͨѶ */ 
	close(sockfd); 
	exit(0); 
} 

static void handle_connection(int sockfd){
    char    sendline[MAXLINE],recvline[MAXLINE];
    int     maxfdp, stdineof;
    struct pollfd pfds[2];
    int n;
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    for (; ;){
        poll(pfds,2,-1);
        if (pfds[0].revents & POLLIN){
            n = read(sockfd,recvline,MAXLINE);
            if (n == 0){
                fprintf(stderr,"client: server is closed.\n");
                exit(1);
            }
            write(STDOUT_FILENO,recvline,n);
        }
        if(pfds[1].revents & POLLIN){
            n = read(STDIN_FILENO,sendline,MAXLINE);
            if (n  == 0){
                shutdown(sockfd,SHUT_WR);
                continue;
            }
            write(sockfd,sendline,n);
        }
    }
}
