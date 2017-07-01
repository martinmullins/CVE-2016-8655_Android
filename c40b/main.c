#include "pagemap.h"
#include "pointer.h"
#include "spawn.h"
#include <stdio.h>


typedef PhyPointer (*ExecuteStep)(PhyPointer);


int main(int argc, char** argv) {

    char buf[100];
    unsigned long kva;
    FILE* fp;
    ExecuteStep e = physmap_page;
    ExecuteStep s = spawn;
    PhyPointer p;
    p.v = 0;
    p.p = 0;

    printf("test spawn:\n");
    p = s(p);
    printf("returned: %p %p\n",p.v, p.p);
    kva = (unsigned long)p.p;
    printf("%lu %p\n", kva, (void*)p.p);
    //fgetc(stdin);
    fp = fopen("/proc/procEntry123","r+");
    fprintf(fp,"%lu",kva);
    while (fscanf(fp,"%s",buf) != EOF) {
        printf("%s", buf);
        fflush(stdout);
    }
    fclose(fp);

    printf("done.\n");
    return 0;
}
