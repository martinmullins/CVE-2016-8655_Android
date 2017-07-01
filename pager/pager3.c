#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define MAP_SIZE 1 // 422  // in pages
int mapsize = MAP_SIZE;
int total = 0;
int cntFails = 10;
#define KERNEL_BOUNDARY 0x23080480 / getpagesize()
#define PAGEMAP_ENTRY 8 //size of an entry of /proc/self/pagemap, the standard is 8 bytes

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
    
int readpagemap(unsigned long virt_addr, PagemapEntry* pe);

int printsz() {
    int i = 0;
    const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double s = total * getpagesize();
    while ( s > 1000 ) {
        s /= 1000;
        i++;
    }
    printf("%.*f %s\n",i,s,units[i]);

}

int mappage() {
    void* addr = mmap(NULL, mapsize*getpagesize(),
            PROT_READ | PROT_EXEC | PROT_WRITE,
            MAP_SHARED | MAP_LOCKED | MAP_POPULATE | MAP_ANONYMOUS,
            -1, 0);
    int rc = 0;
    int ret = (int)addr;
    int pages;
    int cntPresent = 0;
    void* i = 0;
    void* j = 0;

    PagemapEntry pe;

    if (ret != -1) {
        //printf("memory mapped %p\n",addr);
        rc = mlockall(MCL_CURRENT);
        //printf("memory locked %d\n",rc);
        //memset(addr, 0x43, MAP_SIZE);
        cntFails = 30;
    } else {
        printf("mmap failed %d %s\n", ret, strerror(errno));
        return 0;
    }

    for (pages = 0; pages < mapsize; pages++) {
        //printf("p:%d \n",pages);
        i = addr+pages*getpagesize();
        rc = readpagemap((unsigned long)i,&pe);

        if (pe.pfn < KERNEL_BOUNDARY && pe.pfn > 0) {
            printf("\t wtflol %d\n", pe.pfn);
            return pe.pfn;
        }
        if (pe.present) {
            cntPresent++;
            j = memset(i, 0x43, getpagesize());
            if (i != j)
                printf("\t failed on memset\n");
            rc = mlock(i,getpagesize());
            if (rc < 0)
                printf("\t failed on lock\n");

            printf("%d %d ",pe.pfn,KERNEL_BOUNDARY);
            total++;
            printsz();
            return pe.pfn;
        }

        if (rc < 0)
            break;
    }
    return 0;
}

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

int main() {
    int pfn =0;
    while (mappage()) {
    }
    printf("done");
    for(;;) { sleep(1); }
    return 0;
    
}
