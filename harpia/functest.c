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
#include <dlfcn.h>
#include "deviceconfig.h"
#include "lol.h"

typedef void (*fptr)(unsigned long arg);

void* copyfunc(void* ptr) {
    void* result = memcpy(ptr, (const void*)lol_func, lol_func_len);
    fptr f = (fptr)ptr;

    if(result == ptr) {
        printf("muchos succos 1\n");
        fflush(stdout);
    } else { printf("yousuck\n"); fflush(stdout); return 0; }

    unsigned long* addr = malloc(sizeof(unsigned long));
    *addr = 0;
    //char* c = 0;
    //unsigned long l = 0;
    //char* b = (char*)&l;
    //b[0] = '\x25';
    //b[1] = '\x64';
    //b[2] = '\x0a';
    //b[3] = '\x00';

    //printf("%lx\n",l);
    printf("%d <-- before.\n",*addr);
    f((unsigned long)addr);

    printf("%d <-- result?\n",*addr);
    free(addr);

    return 0;
}

static inline void *pageof(const void* p)
{ return (void*)((unsigned long)p & ~(4095));
}

int main(int argc, char** argv) {
    printf("so confused\n");
    void* ptr  = malloc(4096);
    if (mprotect(pageof(ptr), 4096, PROT_READ|PROT_EXEC|PROT_WRITE) == -1) {
        perror("mprotect");
    } else {
        copyfunc( ptr);
    }
    free(ptr);
    return 0 ;
}
