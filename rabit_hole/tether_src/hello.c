#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define RESET_LOC "/data/data/com.example.marto.tether/files/reset.sh"
#define RESET_LOC2 "/data/local/tmp/reset.sh"
void copy_file(const char* fn, const char* fn2) {
    FILE* file = fopen(fn, "r");
    if(!file) {
        return;
    }

    FILE* fileNew;
    int fd;
    if ((fd = open(fn2, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        close(fd);
    }
    printf("Copying..  %s -> %s\n",fn,fn2);
    fflush(stdout);
    fileNew = fopen(fn2,"w");
    printf("Copying..  %s -> %s\n",fn,fn2);
    fflush(stdout);
    for (;;) {
        printf("Copying..  %s -> %s\n",fn,fn2);
        fflush(stdout);
        int c = fgetc(file);
        if ( c != EOF ) {
            fputc(c,fileNew);
            printf("%c",c);
        } else {
            break;
        }
    }
    fclose(fileNew);
    fclose(file);
    printf("Done copy\n");
    fflush(stdout);
}
int main(int argc,char** argv) {
        copy_file(RESET_LOC,RESET_LOC2);
        return 0;
}
