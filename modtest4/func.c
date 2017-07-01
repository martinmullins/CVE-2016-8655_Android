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
#include "mod.h"

const char* lol = "LOLOLOL %d %d\n";

PhyPointer copyfunc(PhyPointer p) {
    printf("Executing copy func...");
    void* result = memcpy(p.v, (const void*)mod_func, mod_func_len);
    printf("\t%p == %p\n",result, p.v);
    result = memcpy(((void*)(((unsigned long)p.v)+1024)), (const void*)lol, 16);
    printf("\t%p == %p\n",result, ((void*)(((unsigned long)p.v)+1024)));
    printf(((char*)(((unsigned long)p.v)+1024)),1,1);
    return p;
}
