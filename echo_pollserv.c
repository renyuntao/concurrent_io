#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<poll.h>
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
#define MAXLINE 100
#define MAX_SOCK 10
#define LISTEN_NUM 5

int main(int argc,char **argv)
{
	int i,maxi,listenfd,connfd,sockfd;
	int nready;
	ssize_t n,m;
	char buf[MAXLINE];
	socklen_t clilen;
	struct pollfd client[MAX_SOCK];
	struct sockaddr_in cliaddr,servaddr;

	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if(listenfd==-1)
	{
		fprintf(stderr,"socket() error\n");
		exit(1);
	}

	memset((void*)&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(8888);

	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1)
	{
		fprintf(stderr,"bind() error\n");
		exit(1);
	}

	if(listen(listenfd,LISTEN_NUM)==-1)
	{
		fprintf(stderr,"listen() error\n");
		exit(1);
	}

	puts("Listening at port 8888");
	puts("Waiting connect...");

	client[0].fd=listenfd;
	client[0].events=POLLRDNORM;
	for(i=1;i<MAX_SOCK;i++)
		client[i].fd=-1;        //-1 indicates available entry
	maxi=0;                  //max index into client array

	for(;;)
	{
		nready=poll(client,maxi+1,-1);
		if(nready==-1)
		{
			fprintf(stderr,"poll() error\n");
			exit(1);
		}

		if(client[0].revents & POLLRDNORM)    //new client connection
		{
			clilen=sizeof(cliaddr);
			connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
			if(connfd==-1)
			{
				fprintf(stderr,"accept() error\n");
				exit(1);
			}

			for(i=1;i<MAX_SOCK;i++)
			{
				if(client[i].fd<0)
				{
					client[i].fd=connfd;    //save descriptor
					break;
				}
			}

			if(i==MAX_SOCK)    //too many descriptor
			{
				printf("Too many client\n");
				exit(1);
			}

			client[i].events=POLLRDNORM;
			if(i>maxi)
				maxi=i;        //max index in client[] array

			if(--nready<=0)
				continue;      //no more readable descriptor
		}

		for(i=1;i<=maxi;i++)     //check all clients for data
		{
			if((sockfd=client[i].fd)<0)
				continue;
			if(client[i].revents & (POLLRDNORM|POLLERR))
			{
				if((n=read(sockfd,buf,MAXLINE))<0)
				{
					if(errno==ECONNRESET)     //connection reset by client
					{
						close(sockfd);
						client[i].fd=-1;
					}
					else
					{
						fprintf(stderr,"read() error\n");
						exit(1);
					}
				}
				else if(n==0)    //connection closed by client 
				{
					close(sockfd);
					client[i].fd=-1;
				}
				else
				{
					//m=write(sockfd,buf,n);
					//if(m==-1)
					if(write(sockfd,buf,n)==-1)
					{
						fprintf(stderr,"write() error\n");
						exit(1);
					}
				}
			}

			if(--nready<=0)
				break;
		}
	}
}
