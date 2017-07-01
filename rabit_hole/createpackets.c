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
            fprintf(stderr,"pad_kmalloc() socket error\n");
            exit(1);
        }

}

int main(int argc, char** argv) 
{
    int fd;
    pad_kmalloc();

    fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ARP));

    if (fd==-1) {
        printf("target socket error\n");
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

    while(1) {
        char x = getc(stdin);
        if (x == 'b') {
            close(fd);
            fprintf(stderr,"target socket freed\n");
            break;
        }
    }

    sleep(13377);
    return 0;
}
