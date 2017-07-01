#include "pagemap.h"
#include "func.h"
#include "pointer.h"
#include <stdio.h>


typedef PhyPointer (*ExecuteStep)(PhyPointer);


int main(int argc, char** argv) {

    char buf[100];
    unsigned long va;
    FILE* fp;
    ExecuteStep e = physmap_page;
    ExecuteStep setfunc = copyfunc;
    PhyPointer p;
    p.v = 0;
    p.p = 0;
    p = e(p);
    printf("%p %p\n", p.v, p.p);
    p = setfunc(p);
    printf("%p %p\n", p.v, p.p);
    va = (unsigned long)p.v;
    printf("%lu %p\n", va, (void*)p.v);
    fp = fopen("/proc/procEntry123","r+");
    fprintf(fp,"%lu",va);
    while (fscanf(fp,"%s",buf) != EOF) {
        printf("%s", buf);
        fflush(stdout);
    }
    fclose(fp);

    //cleanup
    munmap(p.v,getpagesize());

    return 0;
}
