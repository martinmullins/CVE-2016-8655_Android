#include <stdio.h>

void libfun(long arg) {
    long* data = (long*)((void*)arg);
    if (data) {
        *data = 1;
    }
//    printf("OMOMOMOMOMG\n");
    return;
}
