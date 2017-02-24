#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include<arpa/inet.h> //inet_addr

#define BUFLEN 1023
#define DATALEN 8192
#define NUM_THREADS 10

void* lock_ctlbuf(void* threadid) {
    long tid = (long)threadid;
    struct sockaddr_un addr;
    char buf2[100];
    int fd,rc,i;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    union {
        char   control[CMSG_SPACE(BUFLEN)];
        struct cmsghdr cmh;
                        /* Space large enough to hold a ucred structure */
    } control_un;
    struct cmsghdr *cmhp;
    struct msghdr msgh;
    struct iovec iov;
    char data[DATALEN];
     
    //Create socket
    fd = socket(AF_INET , SOCK_STREAM , 0);
    if (fd == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    int sendbuff = 2;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
        perror("Setting sock buff size");
    }
    puts("Socket buffer size set");

     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 5000 );
 
    //Connect to remote server
    if (connect(fd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
    }
     
    printf("Send Msg Test\n");
    memset(&control_un, 'A', sizeof(control_un));
    control_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_RIGHTS;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(data);
    memset(&data, 0x41, sizeof(data));

    //printf("Res: %d %s", res, strerror(errno));
    for ( i = 0; i < 600; ++i) {

        int res = sendmsg(fd, &msgh,0);
        if( res < 0)  {
                perror("sendmsg");
        }
        printf("Sent Message %d\n",i);
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    pthread_t dummy;
    long rc,t;
    for (t = 0; t < NUM_THREADS; t++) {
        rc = pthread_create(&dummy, NULL, lock_ctlbuf, (void*)t);
        if (rc) {
            perror("pthread_create()");
            exit(1);
        }
    }
    pthread_exit(NULL);
    return 0;
}
