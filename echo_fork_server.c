#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<netdb.h>
#include<signal.h>
#include<unistd.h>
#include<error.h>
#include<arpa/inet.h>
#include<sys/un.h>
#include<errno.h>
#include <pthread.h>
#include <mqueue.h>
#define BUF_SIZE 100

void clnt_handler(int clnt_sock);
int main(void)
{
	int serv_sock,clnt_sock;
	struct sockaddr_in serv_addr,clnt_addr;
	socklen_t clnt_len;
	pid_t pid;

	serv_sock=socket(AF_INET,SOCK_STREAM,0);
	if(serv_sock==-1)
	{
		fprintf(stderr,"socket() error\n");
		exit(1);
	}
	memset((void*)&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(8888);

	if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
	{
		fprintf(stderr,"bind() error\n");
		exit(1);
	}

	if(listen(serv_sock,5)==-1)
	{
		fprintf(stderr,"lisen() error\n");
		exit(1);
	}
	puts("Listener on port 8888");
	puts("Waiting for connection...");

	while(1)
	{
		clnt_len=sizeof(clnt_addr);
		clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_len);
		if(clnt_sock==-1)
		{
			fprintf(stderr,"accept() error\n");
			exit(1);
		}
		pid=fork();
		if(pid==0)    //child process
		{
			close(serv_sock);
			if((pid=fork())==0)    //second child process
			{
				clnt_handler(clnt_sock);
			}
			exit(0);             //first child process terminate
		}
		else         //parent process
		{
			wait(NULL);           //wait first child process
			close(clnt_sock);
		}
	}

}

void clnt_handler(int clnt_sock)
{
	char buf[BUF_SIZE];
	memset((void*)buf,0,sizeof(buf));

	write(clnt_sock,"Hello Ren!",strlen("Hello Ren!"));
	while(1)
	{
		read(clnt_sock,buf,BUF_SIZE);
		if(strcmp(buf,"q\n")==0 || strcmp(buf,"Q\n")==0)
			break;
		write(clnt_sock,buf,strlen(buf));
	}
}
