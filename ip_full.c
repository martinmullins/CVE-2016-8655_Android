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
#include<arpa/inet.h> //inet_addr

int main(int argc, char** argv) {
    struct sockaddr_un addr;
    char buf2[100];
    int fd,rc,i;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    fd = socket(AF_INET , SOCK_STREAM , 0);
    if (fd == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 5000 );
 
    //Connect to remote server
    if (connect(fd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    printf("Send Msg Test\n");

    char buf[1024] = {0x41};
    memset(buf,0x41,1023);
    buf[1023]='\0';

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_control = buf;
    msg.msg_controllen = 1024;

    //int res = sendmsg(fd, &msg ,0);

    //printf("Res: %d %s", res, strerror(errno));
    for ( i = 0; i < 100000; ++i) {
        struct goodhdr {
                struct cmsghdr cmh;
                char buf[16];
        } gh;
        struct iovec iov;
        struct msghdr mh;
        struct cmsghdr cmh[2];
        memset(&mh,0,sizeof(mh));
        mh.msg_name = 0;
        mh.msg_namelen = 0;
        mh.msg_iov = &iov;
        mh.msg_iovlen = 1;
        //mh.msg_control = buf;
        //mh.msg_controllen = 1024;
        mh.msg_control = (caddr_t)(struct cmsghdr*)&gh;
        mh.msg_controllen = sizeof(cmh[0]) + sizeof(int);
        //mh.msg_controllen = sizeof(gh.cmh) + sizeof(gh.buf);
        mh.msg_flags = 0;
        iov.iov_base = buf;//"hello";
        iov.iov_len = sizeof(buf);//strlen(iov.iov_base) + 1;
        gh.cmh.cmsg_level = SOL_SOCKET;
        gh.cmh.cmsg_type = SCM_RIGHTS;
        gh.cmh.cmsg_len = sizeof(cmh[0]) + sizeof(int);
        //gh.cmh.cmsg_len = sizeof(gh.cmh) + 1024; //sizeof(gh.buf);
        printf("%d\n",(int)(sizeof(gh.cmh)+sizeof(gh.buf)));
        printf("%d\n",(int)(sizeof(gh.cmh)));
        printf("%d\n",(int)(sizeof(cmh)));
        //*(int *)&cmh[1] = fd;
        //cmh[1] = buf;
        int res =0;
        if ( res=sendmsg(fd,&mh,0) < 0 ) {
            printf("%d %s\n",res,strerror(errno));
        }
        printf("sent");
    }
    return 0;

    while((rc=read(STDIN_FILENO, buf2, sizeof(buf2))) > 0){
        if(write(fd,buf2,rc) != rc) {
            if (rc>0) fprintf(stderr,"partial write");
            else {
                perror("write error");
                exit(-1);
            }

        }
    }
    return 0;
}
