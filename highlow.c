#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HIGH_STR "HighTotal:"
#define LOW_STR "LowTotal:"
int main(int argc, char** argv) {
    //if (argc != 2) { return 2; }
    //!unsigned long  high;
    unsigned long  low;
    char buf1[512];
    int len;
    int check = 0;
    FILE* f = fopen("/proc/meminfo", "r");
    while(fgets(buf1, 255, f) != 0) {
        //!*(buf1+sizeof(HIGH_STR)-1) = '\0';
        //!if (strcmp(HIGH_STR,buf1) == 0) {
        //!    *(buf1+sizeof(HIGH_STR)-1) = ' ';
        //!    check = sscanf(buf1,"%*s %lu %*s", &high);
        //!    //printf("have high str: %s %lu\n",buf1,high);
        //!    continue;
        //!}
        *(buf1+sizeof(LOW_STR)-1) = '\0';
        if (strcmp(LOW_STR,buf1) == 0) {
            //!*(buf1+sizeof(HIGH_STR)-1) = ' ';
            *(buf1+sizeof(LOW_STR)-1) = ' ';
            check = sscanf(buf1,"%*s %lu %*s", &low);
            //printf("have low  str: %s %lu\n",buf1,low);
            break;
        }
    }
    low *= 1000; //kB
    //!high *= 1000; //kB
    //!printf("    HIGH: %lu 0x%lx\n",high,high);
    printf("     LOW: %lu 0x%lx\n",low,low);
    printf("PHYS_MAX: 0x%lx\n",low+0xc0000000);

    
//    buf1[5]=' ';
 //   if (check != 1) {
 //       perror("failed to get btime");
 //   }

    fclose(f);

 //   printf("%d\n", hz);
 //   printf("%lu\n", btime);
 //   printf("%lu\n", (jiffytime/hz)+btime);

    return 0;
}
