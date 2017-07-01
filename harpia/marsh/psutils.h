#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TMPFILE "/data/local/tmp/asdfasdf"
//#define DIRTYCOWLOCATION "/data/data/com.example.marto.tether/files/dirtycow"
#define DIRTYCOWLOCATION "/data/local/tmp/dirtycow"
#define DIRTYCOWLOCATION2 "dirtycow"
#define TCLOCATION "tc"
#define APPPROCESSLOCATION "/system/bin/app_process"
#define PSLOCATION "/system/bin/ps"
#define PIDOFLOCATION "/system/bin/ps"

#ifndef ALOG
#define ALOG(...)  
#endif

int check_ps(const char * exec_path, const char* searchstr,const char* flags,int* psfd) {
    pid_t child_pid, wait_pid;
    int pipefd[2];
    pipe(pipefd);

    child_pid = fork();
    if (0 == child_pid) { //parent
        close(pipefd[0]);
        dup2(pipefd[1],1);
        dup2(pipefd[2],2);
        //close(pipefd[1]);

        int child_ret_code  = 0;
        if( searchstr == NULL) {
           child_ret_code = execlp(exec_path, "pidof", flags, NULL);
        } else {
           child_ret_code = execlp(exec_path, "ps", flags, searchstr, NULL);
        }
        ALOG("Failed to exec child ps");
        return 2;
    } else if (-1 == child_pid) {
        ALOG("Failed to fork\n");
        return 2;
    }

    close(pipefd[1]);
    *psfd = pipefd[0];
    return 0;
}

int get_app_id(int* uid) {
    int psfd;
    int r = check_ps(PSLOCATION,"com.android.systemui","",&psfd);

    int child_status;
    char result[32];
    int pid = (int)getpid();
    if (r || psfd <= 0) {
        return r;
    }

    char buffer;
    short ready=0;
    short n = 0;
    while (read(psfd, &buffer, 1) != 0) {
        if (ready) {
            if (buffer != ' ') {
                result[n] = buffer;
                ++n;
            } else {
               ready = 0;
            } 
        }
        if (buffer == '\n' && !ready) {
            //got first newline, the next string is the context
            ready=1;
        }
    }
    result[n] = '\0';
    //success if n is greater than zero
    if(n > 0) {
        return 0;
    }
    return 2;

}

int get_app_context(char* result) {
    int psfd;
    int r = check_ps(PSLOCATION,"com.android.systemui","-Z",&psfd);

    int child_status;
    int pid = (int)getpid();
    if (r || psfd <= 0) {
        return r;
    }

    char buffer;
    short ready=0;
    short n = 0;
    while (read(psfd, &buffer, 1) != 0) {
        if (ready) {
            if (buffer != ' ') {
                result[n] = buffer;
                ++n;
            } else {
               ready = 0;
            } 
        }
        if (buffer == '\n' && !ready) {
            //got first newline, the next string is the context
            ready=1;
        }
    }
    result[n] = '\0';
    //success if n is greater than zero
    if(n > 0) {
        return 0;
    }
    return 2;

}

int check_running(const char* searchstr) {
    int psfd;
    int r = check_ps(PSLOCATION,searchstr,"",&psfd);

    int child_status;
    int pid = (int)getpid();
    if (r || psfd <= 0) {
        return r;
    }

    char buffer;
    short newlines = 0;
    while (read(psfd, &buffer, 1) != 0) {
        if (buffer == '\n') {
            newlines++;
        }
    }

    //success if two lines, also don't care about waiting for child here
    if(newlines > 1) {
        return 0;
    }
    return 2;
}

int check_for_reset() {
    return check_running("reset");
}

int check_for_app() {
    return check_running(APPPROCESSLOCATION);
}

int check_for_dirtycow2() {
    return check_running(DIRTYCOWLOCATION2);
}


int check_for_dirtycow() {
    return check_running(DIRTYCOWLOCATION);
}

int check_for_tc() {
    return check_running(TCLOCATION);
}

int check_for_bugreport() {
    return check_running("bugreport");
}

int check_for_app_process() {
    int psfd;
    
    int r = check_ps(PIDOFLOCATION,NULL,APPPROCESSLOCATION,&psfd);

    int child_status;
    int pid = (int)getpid();
    if (r || psfd <= 0) {
        return r;
    }

    char buffer;
    short newlines = 0;
    while (read(psfd, &buffer, 1) != 0) {
        if (buffer == '\n') {
            newlines++;
        }
    }

    if(newlines >2) {
        return 0;
    }
    return 2;
}

