#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define MAP_SIZE 128*1024*1024

#include <stdint.h>
#include <errno.h>
#define PAGEMAP_ENTRY 8
#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF
#define KERNEL_BOUNDARY 0xC0000000

const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )
int read_pagemap(unsigned long virt_addr);


typedef struct {
    unsigned int pfn; //32 bits is OK for my pagesize and ram
    unsigned int notused : 23; //not used
    //unsigned short softDirty : 1; //
    //unsigned short exclusive : 1;
    //unsigned short zero : 3;
    unsigned short pageshift : 6; //pre 3.11
    unsigned short shared : 1;
    unsigned short swapped : 1;
    unsigned short present : 1;
} PagemapEntry;
 
int readpagemap(unsigned long virt_addr);

void* mappage() {
    void* addr = mmap(NULL, MAP_SIZE/getpagesize(), PROT_READ | PROT_EXEC | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE | MAP_ANONYMOUS, -1, 0);
    int rc = 0;
    if (addr) {
        memset(addr, 0x43, getpagesize());
        printf("memory mapped 0x%p\n",addr);
        rc = mlockall(MCL_CURRENT);
        printf("memory locked %d\n",rc);
   }
    read_pagemap((unsigned long)addr);
    readpagemap((unsigned long)addr);
    read_pagemap((unsigned long)addr+getpagesize());
    readpagemap((unsigned long)addr+getpagesize());
    printf("..\n");
    printf("..\n");
    return addr;
}

int readpagemap(unsigned long virt_addr) {
   FILE * f;
   uint64_t file_offset;
   int status;
   PagemapEntry pe;
   int one;
   int two;
   int i;
   char buf[8];
   int c;

   f = fopen("/proc/self/pagemap", "rb");
   if(!f){
      printf("Error! Cannot open %s\n", "/proc/self/map");
      return -1;
   }

   file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;
   printf("Reading at 0x%llx\n",  (unsigned long long) file_offset);
   status = fseek(f, file_offset, SEEK_SET);
   if(status){
      perror("Failed to do fseek!");
      return -1;
   }

fread(&pe, sizeof(PagemapEntry),1,f);
//fread(&one, sizeof(one), 1, f);
 //  fread(&two, sizeof(two), 1, f);
   
   //for(i=0; i < PAGEMAP_ENTRY; i++){
   //   c = getc(f);
   //   printf("[%d]0x%x ", i, c);
   //}
   printf(">>PFN: 0x%x %d present=%d swapped=%d pageshift=%d\n",pe.pfn, pe.pfn,pe.present,pe.swapped,pe.pageshift);
   return pe.pfn;
}

int read_pagemap(unsigned long virt_addr){
   FILE * f;
    int i, c, pid, status;
    uint64_t read_val, file_offset;
    char *end;
    int present, swapped,pfn;

   printf("Big endian? %d\n", is_bigendian());
   f = fopen("/proc/self/pagemap", "rb");
   if(!f){
      printf("Error! Cannot open %s\n", "/proc/self/map");
      return -1;
   }
   
   //Shifting by virt-addr-offset number of bytes
   //and multiplying by the size of an address (the size of an entry in pagemap file)
   file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;
   printf("Vaddr: 0x%lx, Page_size: %d, Entry_size: %d\n", virt_addr, getpagesize(), PAGEMAP_ENTRY);
   printf("Reading at 0x%llx\n",  (unsigned long long) file_offset);
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
      printf("[%d]0x%x ", i, c);
   }
   for(i=0; i < PAGEMAP_ENTRY; i++){
      //printf("%d ",c_buf[i]);
      read_val = (read_val << 8) + c_buf[i];
   }
   printf("\n");
   printf("Result: 0x%llx\n", (unsigned long long) read_val);
   present = GET_BIT(read_val, 63);
   swapped = GET_BIT(read_val, 62);
   pfn = (int)GET_PFN(read_val);
   if(GET_BIT(read_val, 63)) {
      printf("PFN: 0x%llx %d present=%d swapped=%d\n",(unsigned long long) GET_PFN(read_val),pfn,present,swapped);
      fclose(f);
      return (unsigned long long) GET_PFN(read_val);
      
   } else {
      printf("Page not present\n");
   }
   if(GET_BIT(read_val, 62)) {
      printf("Page swapped\n");
   }
   fclose(f);
   return 0;
}


int main() {
    void* addr1;
    void* addr2;
    printf("%d\n",getpagesize());
    addr1 = mappage();
    addr2 = mappage();
    for(;;) { sleep(1); }
    return 0;
}
