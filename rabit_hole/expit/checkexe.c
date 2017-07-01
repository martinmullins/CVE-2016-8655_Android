#include "pagemap.h"
#include "func.h"
#include "pointer.h"
#include <stdio.h>
#include "checkexe.h"

PhyPointer checkexe(PhyPointer p) {
    char buf[100];
    unsigned long va;
    FILE* fp;
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
    return p;
}
