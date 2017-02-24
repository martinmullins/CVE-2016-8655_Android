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
char code[]  = {'\x1e','\xff','\x2f','\xe1'};

typedef union  { void* ptr; long* lptr; long l; } helper;

void lolrofl() {
    int k = 32;
    ++k;
    return;
}

int main(int argc, char** argv) {
    int diff = ((const void*)&main - (const void*)&lolrofl);
    printf("%x\n", (*((long*)code)));
    void* memfun  = malloc(4096);
    if (mprotect(memfun, 4096, PROT_READ|PROT_EXEC|PROT_WRITE) == -1) {
        perror("mprotect");
    }
    memcpy(memfun, (const void*)&lolrofl, diff);
    (*(void(*)())memfun) ();
    free(memfun);
    return 0 ;
    typedef void (*_func_ptr)(long arg);
    char* con = 0;
    void *handle = dlopen("libmytestlib.so",RTLD_LAZY);
    int fd;
    void* data;
    struct stat s;
    fd = open("libmytestlib.so", O_RDWR);
    int pagesize;
    pagesize = getpagesize();
    if (fstat(fd, &s) < 0) {
        perror("asdf");
        exit(-1);
    }
    helper h;

    data = mmap((void*)0x50000000, s.st_size, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    h.ptr = mmap(NULL, sizeof(long), PROT_READ | PROT_EXEC | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!h.ptr) {
        printf("%s\n",strerror(errno));
        fflush(stdout);
        perror("global bad");
        exit(-1);
    }
    printf("can mmap?\n");
    fflush(stdout);
    *(h.lptr) = 0;
    printf("GLOBAL IS SET %d, %p %x\n", (*h.lptr), h.ptr, h.l);
    fflush(stdout);



    if (handle) {
        _func_ptr _func = dlsym(handle, "libfun");
        printf("%p\n" , (void*)_func);
        unsigned long* other =(void*)((char*)data+0x31c); 
        _func_ptr _lol = (void*)((char*)data+0x31c); 
        //printf("--> %p %p\n", data, other);
        //unsigned long val1 = *((unsigned long*)_func);
        //unsigned long val2 = *(other);
        //printf("--> %x %x\n", val1, val2);
        //if(_func) {
        //    _func(h.l);
        //    printf("Asdf2\n");
        //} else { printf("Bad Fuc PTr\n"); }
        if(_lol) {
            _lol(h.l);
          printf("Asdf\n");
        } else { printf("Bad Fuc LOL PTr\n"); }
        
    } else { printf("Bad Handle\n"); }
    fflush(stdout);

    if (data) {
        munmap(data, pagesize);
    }
    printf("GLOBAL IS GET %d\n", *(h.lptr));
    fflush(stdout);
    //TODO not requred for MAP_ANONYMOUS?
    //if (h.ptr) {
    //    munmap(h.ptr, sizeof(long));
    //    printf("free error %s\n",strerror(errno));
    //    fflush(stdout);
    //    exit(-1);
    //}

    return 0;
}
