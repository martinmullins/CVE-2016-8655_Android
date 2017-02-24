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
#include <sys/mman.h>


#define PAD 64
#define NUMKEYS 100
//int  ad_fds[PAD];
#define KEYSERIAL 123456

#define CONF_RING_FRAMES 1
#define MMAP_SIZE 1024

struct tpacket_req3 tp;
int sfd;
int mapped = 0;

void testkeyctl() {
    //int res = syscall(__NR_keyctl, 0, KEYSERIAL, 1);
    int res = syscall(__NR_keyctl, 1, "yolo");
    printf("kyctl: %d :: %s\n",res,strerror(errno));
    fflush(stdout);
    int res2 = syscall(__NR_keyctl, 1, -2);
    printf("kyctl: %d :: %s\n",res2,strerror(errno));
    fflush(stdout);

    char buffer[4096];
    buffer[0] = '\0';
    long dat = syscall(__NR_keyctl, 6, res, buffer, 4096);
    printf("kyctl: %d :: %s\n",dat,strerror(errno));
    printf("kyctl: %d :: %s\n",dat,buffer);
    fflush(stdout);

    int res3 = syscall(__NR_add_key, "user","lol",buffer,20,res);
    if (res3 < 0) {
        printf("err add_key: %d %s\n",res3,strerror(errno));
        fflush(stdout);
        return;
    }
    printf("added key: %d to keyring %d\n",res3,res);
    fflush(stdout);


}

void addkey(void* exploitbuf, short len) {
    int res = 0;
    char* a = "desc";
    int keyring = syscall(__NR_keyctl, 0, "asdf",1);
    if (keyring < 0) {
        printf("err add_key: %d %s\n",keyring,strerror(errno));
        fflush(stdout);
        return;
    }
    printf("keyring %d\n",keyring);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,keyring);
    if (res < 0) {
        printf("err add_key: %d %s\n",res,strerror(errno));
        fflush(stdout);
        return;
    }
    printf("added key: %d to keyring %d\n",res,keyring);
    fflush(stdout);
}

void testkey(void* exploitbuf, short len) {
    int res = 0;
    char* a = "desc";

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-1);
    printf("Key -1 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-2);
    printf("Key -2 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-3);
    printf("Key -3 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-4);
    printf("Key -4 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-5);
    printf("Key -5 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-6);
    printf("Key -6 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-7);
    printf("Key -7 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);

    res = syscall(__NR_add_key, "user",a,exploitbuf,len,-8);
    printf("Key -8 Added: %d %s\n",res,strerror(errno));
    fflush(stdout);
}

void testkeyid(int id, void* exploitbuf, short len) {
    int res = 0;
    char* a = "descasf";

    res = syscall(__NR_add_key, "user",a,exploitbuf,5,-1*id);
    printf("Key %d Added: %d %s\n",-1*id,res,strerror(errno));
    fflush(stdout);
}

void kmalloc(void* exploitbuf, short len)
{
    unsigned int i =0;
    //void* res = 0;
    int res = 0;
    for(i=0;i<NUMKEYS;++i) {
        char a[2];
        a[0] = (char)i;
        a[1] = '\0';
        res = syscall(__NR_add_key, "user",a,exploitbuf,len,-2);
        //res = add_key("user","wtf",exploitbuf,len,-2);
        //res = mmap(NULL, MMAP_SIZE,
        //           PROT_READ | PROT_WRITE | PROT_EXEC,
        //          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        printf("Key Added: %d %s\n",res,strerror(errno));
        fflush(stdout);
    }
    printf("Allocated %d Maps\n", NUMKEYS);
    fflush(stdout);
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

int main(int argc, char** argv) 
{
    int fd;
    char buffer[1024] = {0xFF};

    //kmalloc(buffer,1024);
    testkeyid(argc,buffer,1024);
    testkeyid(argc,buffer,1024);
    //addkey(buffer,1024);
    //testkeyctl();

    sleep(1337);
    return 0;
}
