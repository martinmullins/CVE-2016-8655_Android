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
#include "func.h"
#include "deviceconfig.h"

void lolrofl(unsigned long arg) {
    char* c = 0;
    int k = 32;
    unsigned long l = arg;
    //int* g2 = (int*)arg;
    //*g2=1;
    ++k;
    typedef int (*printf_ptr)( const char* format, ... );
    printf_ptr p = (printf_ptr)KSYM_PRINTK;
    c = (char*)&l;
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    p(c,123);
    return;
}

PhyPointer copyfunc(PhyPointer p) {
    printf("Executing copy func...");
    int diff = ((const void*)&copyfunc - (const void*)&lolrofl);
    printf("\t%d\n",diff);
    int* g = malloc(sizeof(int));
    *g = 0;
    void* result = memcpy(p.v, (const void*)&lolrofl, diff);
    printf("\t%p == %p\n",result, p.v);
    //(*(void(*)(unsigned long arg))p.v) ((unsigned long)g);
    //printf("g= %d\n", *g);
    free(g);
    fflush(stdout);
    return p;
}
