#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<string.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<pthread.h>
#define  SERVERPORT  3333
#define  BACKLOG 10
#define  MAX_CONNECTRD_NO  10
#define  MAXDATASIZE  5
 struct  sockaddr_in  server_sockaddr,client_socketaddr;
   int  sin_size,sendbytes;
   int  sockfd,client_fd;
   fd_set  readfd, writefd;
   char  buf[MAXDATASIZE]="happy";
void thread1(void)
{   

printf("thread1===>client_fd=%d\n",client_fd);
   if((sendbytes=send(client_fd,buf,5,0))==-1){
		perror("send");
		exit(1);
	}
	printf("sendbytes=%d\n",sendbytes);
  pthread_exit(0);
}
  
int main()
{ 
   pthread_t id1,ret;
   if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
   	{perror("socket"),exit(1);}
   printf("socket  sucess!sockfd=%d\n",sockfd);
   server_sockaddr.sin_family=AF_INET;
   server_sockaddr.sin_port=htons(SERVERPORT);
   server_sockaddr.sin_addr.s_addr=INADDR_ANY;
   bzero(&(server_sockaddr.sin_zero),8);

   if((bind (sockfd,(struct sockaddr*)&server_sockaddr,sizeof(struct  sockaddr)))==-1)
   	{
        perror("bind" );
		exit(1);
   }
   if((listen(sockfd,BACKLOG))==-1)
   	{
		perror("listen");
		exit(1);
   	}
   printf("listening......\n");
    FD_ZERO(&readfd);
	 FD_SET(sockfd,&readfd);
   while(1)
   	{   
     sin_size=sizeof(struct sockaddr_in);
	   
              if(select(MAX_CONNECTRD_NO,&readfd,NULL,NULL,(struct timeval *)0)>0)
          	{     
          	    printf("secect----------->\n");
			if(FD_ISSET(sockfd,&readfd)>0)
			{   
  		  sin_size=sizeof(struct sockaddr_in);
		  printf("listening......\n");
	   		if((client_fd=accept(sockfd,(struct sockaddr *)&client_socketaddr,&sin_size))==-1)
			   	{
				  perror("accept\n");
				  exit(1);
			      }
				  printf("accept sucess\n");
		   	 }

    		 printf("access--->client_fd=%d\n",client_fd);
				//ret=pthread_create(&id1,NULL,(void *)thread1,(void *)&client_fd);
				ret=pthread_create(&id1,NULL,(void *)thread1,NULL);
				     if(ret!=0)
				     	{  perror("create pthread error!");
					   exit(1);
				     	}
					 printf("ret=%d\n",ret);
		           pthread_join(id1,NULL);			 
			      }
		  } 
close(sockfd);		  
}  
