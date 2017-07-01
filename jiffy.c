#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char* btime_str = "btime";
int main(int argc, char** argv) {
    if (argc != 2) { return 2; }
    unsigned long jiffytime = atol(argv[1]);
    unsigned long btime;
    char buf1[512];
    int check = 0;
    int hz = sysconf(_SC_CLK_TCK);
    FILE* f = fopen("/proc/stat", "r");
    while(fgets(buf1, 255, f) != 0) {
        buf1[5]='\0';
        if (strcmp(buf1,btime_str) == 0) {
            break;
        }
    }
    
    buf1[5]=' ';
    check = sscanf(buf1,"%*s %lu", &btime);
    if (check != 1) {
        perror("failed to get btime");
    }

    fclose(f);

    printf("%d\n", hz);
    printf("%lu\n", btime);
    printf("%lu\n", (jiffytime/hz)+btime);

    return 0;
}
