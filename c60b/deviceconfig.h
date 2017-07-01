#ifndef __DEVICECONFIG_H
#define __DEVICECONFIG_H

#define DATALEN 8192
#define SK_PAD 64
#define SPRAY_THREAD_COUNT 300
#define CONF_RING_FRAMES 1
 
//see end of file for boundary number
#define KERNEL_BOUNDARY 0x23080480
//#define PHYS_MAX 0xec8fa760
//ummm this should be 0x8
#define PAGE_OFFSET 0x80000000 //in userspace this boundary is offset by PAGE_OFFSET
#define PHYS_MIN PAGE_OFFSET
#define PHYS_MAX PAGE_OFFSET + KERNEL_BOUNDARY

//
// harpia real
//
#ifdef HARP

#define KSYM_BOOT_TVEC_BASE 0xc04fb100
#define KSYM_PRINTK 0xc0360ae8

#define BUFLEN 1088 //packet_sock size that will be kmalloced
#define TIMER_OFFSET 648 // 72+40+536, struct tpacket_..,packet_ring_buffer,packet_sock+sk

struct timer_list {
        int next;
        int prev;
	//struct list_head           entry;                /*     0     8 */
	long unsigned int          expires;              /*     8     4 */
	unsigned int base;//struct tvec_base *         base;                 /*    12     4 */
	void                       (*function)(long unsigned int); /*    16     4 */
	long unsigned int          data;                 /*    20     4 */
	int                        slack;                /*    24     4 */
	int                        start_pid;            /*    28     4 */
	void *                     start_site;           /*    32     4 */
	char                       start_comm[16];       /*    36    16 */

	/* size: 52, cachelines: 1, members: 9 */
	/* last cacheline: 52 bytes */
};
//
// end harpia
//

#else

//
// goldfish sim
//

#define KSYM_BOOT_TVEC_BASE 0xc04fb100
#define KSYM_PRINTK 0xc0360ae8

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
//
// end goldfish
//


#endif


//
// Moto
//
// Kernel Boundary
// Lowmem can be found for a device by running /system/bin/bugreport
// adb shell bugreport >bug.log
// then finding the following:
//
//------ MEMORY INFO (/proc/meminfo) ------
//MemTotal:        1960336 kB
//MemFree:          143920 kB
//Buffers:           23804 kB
//Cached:          1314540 kB
//SwapCached:            0 kB
//Active:           572036 kB
//Inactive:        1068124 kB
//Active(anon):     203296 kB
//Inactive(anon):   101208 kB
//Active(file):     368740 kB
//Inactive(file):   966916 kB
//Unevictable:         996 kB
//Mlocked:               0 kB
//HighTotal:       1372608 kB
//HighFree:          52764 kB
//LowTotal:         587728 kB <--- this is physical mapped kernel memory
//LowFree:           91156 kB
//SwapTotal:        262140 kB
//SwapFree:         262068 kB
//Dirty:                60 kB
//Writeback:             0 kB
//AnonPages:        302728 kB
//Mapped:           244980 kB
//Shmem:              1808 kB
//Slab:              46292 kB
//SReclaimable:      18676 kB
//SUnreclaim:        27616 kB
//KernelStack:        7752 kB
//PageTables:        15640 kB
//NFS_Unstable:          0 kB
//Bounce:                0 kB
//WritebackTmp:          0 kB
//CommitLimit:     1242308 kB
//Committed_AS:   12911708 kB
//VmallocTotal:     409600 kB
//VmallocUsed:       15268 kB
//VmallocChunk:     137244 kB
//
//
// Then the kernel boundary is definitely under 587728*1000 B = 587728000 = 0x23080480
// Pages are mapped for kernel source and kernel pages tables, but we will ignore those for now.
// userspaced mapped page needs to be under this location
//
