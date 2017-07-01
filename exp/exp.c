#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include "pointer.h"
#include "pagemap.h"
#include "func.h"
#include "race.h"

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

ExecuteStep stateMachine[] = {
    stepOne,
    physmap_page,
    stepTwo,
    copyfunc,
    race,
    stepThree };

int main(int argc, char** argv) {
    int i = 0;
    PhyPointer result;
    result.p = 0;
    result.v = 0;
    for (i = 0; i < sizeof(stateMachine)/sizeof(ExecuteStep); ++i) {
        result = stateMachine[i](result);
        if (!result.v) {
            break;
        }
    }
    return 0;
}

