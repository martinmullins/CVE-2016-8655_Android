#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/memcontrol.h>
#include <asm/pgtable.h>
#include <linux/kallsyms.h>
#include <linux/elf.h>
#include <linux/gfp.h>
#include <linux/kernel_stat.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/ksm.h>
#include <linux/rmap.h>
#include <linux/export.h>
#include <linux/delayacct.h>
#include <linux/init.h>
#include <linux/writeback.h>
#include <linux/memcontrol.h>
#include <linux/mmu_notifier.h>
#include <linux/kallsyms.h>
#include <linux/swapops.h>
#include <linux/elf.h>
#include <linux/gfp.h>

#include <asm/io.h>
#include <asm/pgalloc.h>
#include <asm/uaccess.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>


#define MAX_LEN       4096
int read_info( char *page, char **start, off_t off,int count, int *eof, void *data );
ssize_t write_info( struct file *filp, const char __user *buff,unsigned long len, void *data );
static unsigned long justexec(unsigned long user_addr);

static struct proc_dir_entry *proc_entry;
static char *info;
static int write_index;
static int read_index;
static unsigned long address;
//static struct mm_struct *__init_mm;

int init_module( void )
{
    int ret = 0;
    info = (char *)vmalloc( MAX_LEN );
    memset( info, 0, MAX_LEN );
    proc_entry = create_proc_entry( "procEntry123", 0644, NULL );

    printk("INIT_MM is %p\n",(void*)&init_mm);
    //__init_mm = (struct mm_struct *)kallsyms_lookup_name("init_mm");
    //if(!__init_mm) {
    //    printk("FAILEIANEFIAWNFIAWNEFIAWNEFIANWEFIN\n");
    //    vfree(info);
    //    return -1;
    //}
    if (proc_entry == NULL)
    {
        ret = -1;
        vfree(info);
        printk(KERN_INFO "procEntry123 could not be created\n");
    }
    else
    {
        write_index = 0;
        read_index = 0;
        proc_entry->read_proc = read_info;
        proc_entry->write_proc = write_info;
        printk(KERN_INFO "procEntry123 created.\n");
    }

    return ret;
}

void cleanup_module( void )
{
    remove_proc_entry("procEntry123", proc_entry);
    printk(KERN_INFO "procEntry123 unloaded.\n");
    vfree(info);
}



ssize_t write_info( struct file *filp, const char __user *buff, unsigned long len, void *data )
{
    char* endp = 0;
    int capacity = (MAX_LEN-write_index)+1;
    unsigned int oldindex = 0;
    if (len > capacity)
    {
        printk("No space to write in procEntry123!\n");
        return -1;
    }
    if (copy_from_user( &info[write_index], buff, len ))
    {
        printk("iCasdinfaisdfnpace to write in procEntry123!\n");
        return -2;
    }

      
    oldindex=write_index;
    write_index += len;
    info[write_index] = 0;
    address = simple_strtoul(info+oldindex,&endp,10);
    printk("Read String: %s\n", info+oldindex);
    printk("Read Long: %ld\n", address);
    printk("VA: %p\n", (void*)address);
    //printk("This is a number %lu\n",simple_strtol(info,&endp,10));
    return len;
}

int read_info( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    int len;
    unsigned long phys_addr;
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    if (read_index >= write_index)
    read_index = 0;

    //len = sprintf(page, "%s\n", &info[read_index]);

    printk("execuing %lx\n",address);
    phys_addr = justexec(address);
    len = sprintf(page, "Address Supplied %lu\n", address);
    //len += sprintf(page, "user: %lux  phys: %lux\n", address, phys_addr);
    read_index += len;
    return len;
}

static unsigned long justexec(unsigned long user_addr)
{
    typedef void (*func)(unsigned long arg);
    typedef int (*__trace_printk_ptr)(unsigned long ip, const char *fmt, ...);
    __trace_printk_ptr t = (__trace_printk_ptr)0xc007a48c;
    func f;
    printk("Executing page 0x%lx\n",user_addr);
    printk("executing page....\n");
    f = (func)user_addr;
    printk("%p\n",(void*)f);
    //printk((const char*)((unsigned long)((void*)f) + 1024),0,0);
    f(user_addr);
    printk("Done exec..\n");
    t(0,"Can we call dis now exec..\n");

    return 0;
}
MODULE_LICENSE("GPL");
