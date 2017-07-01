#ifndef __DEVICECONFIG_H
#define __DEVICECONFIG_H
//
// goldfish sim
//
#define KSYM_BOOT_TVEC_BASE 0xc04fb100
#define KSYM_PRINTK 0xc0360b88
//c0360794
//c0360790 
#define DATALEN 8192
#define SK_PAD 64
#define SPRAY_THREAD_COUNT 100
#define CONF_RING_FRAMES 1
#define BUFLEN 776 //sk obj size that will be kmalloced
#define TIMER_OFFSET 488 // timer struct offset in sk object // 72+32+384
#endif
