#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define RESET_LOC "/data/data/com.example.marto.tether/files/reset.sh"
#define RESET_LOC2 "/data/local/tmp/reseta.sh"

int main(int argc,char** argv) {
        char * argv2[]={RESET_LOC,RESET_LOC,NULL};
        char * envp2[]={0,NULL};
        int rc = execve("/system/bin/sh",argv2,envp2);
        printf("Success?\n");
        fflush(stdout);
        return 0;
}
