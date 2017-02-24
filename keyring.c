#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <stdlib.h>
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

#define KEY_POS_ALL 0x3f000000
#define KEY_USR_ALL 0x003f0000
//#include <android/log.h>
//#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define BLOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define LOG_TAG "asdf"

#define ALOG(...) printf("%s\n",__VA_ARGS__); fflush(stdout);

#include "psutils.h"

#define HOST_NAME "app_process32"
#define CONTEXT_APP "u:r:platform_app:s0:c512,c768"
#define CONTEXT_ZYG "u:r:zygote:s0"
#define CONTEXT_SHL "u:r:shell:s0"
#define CONTEXT_ZYGAPP "u:object_r:zygote_exec:s0"
#define CONTEXT_SYS "u:r:system_server:s0"
#define DATAXML "/data/data/com.android.systemui/shared_prefs/com.android.systemui.xml"
#define RESCACHE "/data/resource-cache/test123"
//#define STEP1_FILE "/data/resource-cache/step1_A"
//#define STEP2_FILE "/data/resource-cache/step2_A"
#define STEP1_FILE "/data/local/tmp/step1_A"
#define STEP2_FILE "/data/local/tmp/step2_A"
#define DONE_FILE "/data/local/tmp/done"
#define RUNONCE_FILE "/data/local/tmp/test1234"
//#define STEP1_FILE "/data/local/tmp/test1234"
#define NEWDIR "/data/resource-cache/"
//#define APPPROBK "/data/local/tmp/app_process32old2"
#define APPPROBK "/system/bin/tc"
#define NEWXML "/data/local/tmp/com.android.systemui.xml"
#define DATADIR "/data/data/com.android.systemui/files"
#define DATADIR2 "/data/data/com.android.systemui/"
#define DATADIR3 "/data/data/com.android.systemui/shared_prefs/"
#define DATADIR4 "/data/data/com.android.systemui/code_cache/"
#define DATADIR5 "/data/data/com.android.systemui/cache/"
#define APPID 10067
#define THEID 10067
#define ROOTID 0
#define SYSID 1000
#define SHELLID 2000
#define OBJ1 "/data/data/com.android.systemui/files/boot.ad"
#define OBJ2 "/data/data/com.android.systemui/files/boot.ad.image"
#define OBJ3 "/data/data/com.android.systemui/files/boot.test"

typedef char* security_context_t;
void set_key_context(char* ctx) {
    typedef int (*_setkeycreatecon_ptr)(char* con);
    typedef int (*_freecon_ptr)(char* con);

    char* con = 0;
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {
        _setkeycreatecon_ptr _setkeycreatecon = dlsym(handle, "setkeycreatecon");
        _freecon_ptr _freecon = dlsym(handle, "freecon");
        if(_setkeycreatecon) {
            int ret = _setkeycreatecon(ctx);
            if(ret == 0) {
                ALOG("SET CONTEXT %s",ctx);
                fflush(stdout);
            } else {
                ALOG("faield to set context\n");
            }
        } else {
            ALOG("faield to set fn ptr setkeycreatecon(conn);\n");
        }
    } else {
        ALOG("failed to get selinux lib handle\n");
    }
}

void get_key_context() {
    typedef int (*_getkeycreatecon_ptr)(char** con);
    typedef int (*_freecon_ptr)(char* con);

    char* con = 0;
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {
        _getkeycreatecon_ptr _getkeycreatecon = dlsym(handle, "getkeycreatecon");
        _freecon_ptr _freecon = dlsym(handle, "freecon");
        char* con = 0;
        if(_getkeycreatecon) {
            int ret = _getkeycreatecon(&con);
            if(ret == 0) {
                printf("Have context: %s\n",con);
                fflush(stdout);
                if (_freecon) {
                    _freecon(con);
                }
            } else {
                ALOG("faield to get context\n");
            }
        } else {
            ALOG("faield to get fn ptr getkeycreatecon(conn);\n");
        }
    } else {
        ALOG("failed to get selinux lib handle\n");
    }
}

int main(int argc, char** argv) {
    printf("Getting context\n");
    get_key_context();
    set_key_context(CONTEXT_SHL);
    get_key_context();
    return 0;

}
