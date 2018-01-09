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
#define OPEN_MAX 1000
#define MAXLINE     1024

static void handle_connection(struct pollfd *connfds,int num);

int main(int argc, char *argv[]) 
{ 
	int sockfd,connfd; 
	struct sockaddr_in server_addr; 
	struct sockaddr_in client_addr; 
	socklen_t cliaddrlen; 
	int nbytes;
	char buffer[1024];
	
	int maxi, nready, i;
	struct pollfd clientfds[OPEN_MAX];
	

	/* �������˿�ʼ����sockfd������ */ 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 

	/* ����������� sockaddr�ṹ */ 
	bzero(&server_addr,sizeof(struct sockaddr_in)); // ��ʼ��,��0
	server_addr.sin_family=AF_INET;                 // Internet
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // (���������ϵ�long����ת��Ϊ�����ϵ�long����)���κ�����ͨ��  //INADDR_ANY ��ʾ���Խ�������IP��ַ�����ݣ����󶨵����е�IP
	//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1");  //���ڰ󶨵�һ���̶�IP,inet_addr���ڰ����ּӸ�ʽ��ipת��Ϊ����ip
	server_addr.sin_port=htons(portnumber);         // (���������ϵ�short����ת��Ϊ�����ϵ�short����)�˿ں�
	
	/* ����sockfd��������IP��ַ */ 
	if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 

	/* �����������ӵ����ͻ����� */ 
	if(listen(sockfd,5)==-1) 
	{ 
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	
	clientfds[0].fd = sockfd;
	clientfds[0].events = POLLIN;
	
	for(i=1; i<OPEN_MAX; i++){
		clientfds[i].fd = -1;
	}
	
	maxi = 0;
	
	
	while(1) 
	{ 
		nready = poll(clientfds, maxi+1, -1);
		
		if(nready == -1){
			fprintf(stderr, "poll error��%s\n\a", strerror(errno));
			exit(1);
		}
		if(clientfds[0].revents & POLLIN){
			cliaddrlen=sizeof(struct sockaddr_in); 
			if((connfd=accept(sockfd,(struct sockaddr *)(&client_addr),&cliaddrlen))==-1) 
			{ 
				fprintf(stderr,"Accept error:%s\n\a",strerror(errno)); 
				exit(1); 
			} 
			fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr)); // �������ַת����.�ַ���
			
			for(i=0; i<OPEN_MAX; i++){
				if(clientfds[i].fd == -1){
					clientfds[i].fd = connfd;
					break;
				}
			}
			
			if(i == OPEN_MAX){
				fprintf(stderr, "too many clients\n");
				exit(1);
			}
			clientfds[i].events = POLLIN;
			maxi = (i > maxi ? i : maxi);
			
			if(--nready <= 0)
				continue;
		}
		/* ����������,ֱ���ͻ����������� */ 
		handle_connection(clientfds, maxi);
	} 

	/* ����ͨѶ */ 
	close(sockfd); 
	exit(0); 
} 
static void handle_connection(struct pollfd *connfds,int num){
    int i,n;
    char buf[MAXLINE];
    memset(buf,0,MAXLINE);

    for(i=0; i<= num; i++){
        if(connfds[i].fd < 0)
            continue;

        if(connfds[i].revents & POLLIN){
            n = read(connfds[i].fd, buf, MAXLINE);
            if(0 == n){
                close(connfds[i].fd);
                connfds[i].fd = -1;
                continue;
            }
            write(STDOUT_FILENO, buf, n);

            write(connfds[i].fd, buf, n);
        }
    }
}