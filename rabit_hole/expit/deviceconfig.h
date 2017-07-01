#ifndef __DEVICECONFIG_H
#define __DEVICECONFIG_H
//
// goldfish sim
//
#define KSYM_BOOT_TVEC_BASE 0xc04fb100
//#define KSYM_PRINTK 0xc0360794
#define KSYM_PRINTK 0xc0360b88
//c0360794
//c0360790 
#define DATALEN 8192
#define SK_PAD 64
#define SPRAY_THREAD_COUNT 100
#define CONF_RING_FRAMES 1
#define BUFLEN 776 //sk obj size that will be kmalloced
#define TIMER_OFFSET 488 // timer struct offset in sk object // 72+32+384
struct timer_list {
        int next;
        int prev;
        //struct list_head           entry;                /*     0     8 */
        long unsigned int          expires;              /*     8     4 */
        unsigned int base;//struct tvec_base *         base;                 /*    12     4 */
        void                       (*function)(long unsigned int); /*    16     4 */
        long unsigned int          data;                 /*    20     4 */
        int                        slack;                /*    24     4 */

        /* size: 28, cachelines: 1, members: 6 */
        /* last cacheline: 28 bytes */
};


#endif
