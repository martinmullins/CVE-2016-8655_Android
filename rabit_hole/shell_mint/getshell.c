#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#define _GNU_SOURCE
#include <fcntl.h>


pthread_t tid[2];

void* copyFd(void *arg)
{
    unsigned long i = (long)arg;
    pthread_t id = pthread_self();
    int sockfd = i;
    size_t n = 0;

    if(pthread_equal(id,tid[0]))
    {
      while (n=splice(0, NULL,sockfd,NULL,256, 0)){
	;
	}
    }
    else
    {
      while (n=splice(sockfd, NULL,1,NULL,256, 0)){
	;
	}
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    int i = 0;
    int err;
    int sockfd = 0, n = 0;
    unsigned long sockfdlu = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 


    sockfdlu = sockfd;
    pthread_join(tid, NULL);
    while(i < 2)
    {
      err = pthread_create(&(tid[i]), NULL, &copyFd, (void*)sockfd);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");

        i++;
    } 

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    return 0;
}
