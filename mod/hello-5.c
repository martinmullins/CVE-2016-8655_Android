#include <linux/module.h>
#include <linux/kernel.h>
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
static unsigned long user_to_phys(unsigned long user_addr);
void follow_pte(struct mm_struct * mm, unsigned long address);

static struct proc_dir_entry *proc_entry;
static char *info;
static int write_index;
static int read_index;
static unsigned long address;

int init_module( void )
{
    int ret = 0;
    info = (char *)vmalloc( MAX_LEN );
    memset( info, 0, MAX_LEN );
    proc_entry = create_proc_entry( "procEntry123", 0644, NULL );

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
        printk(KERN_INFO "No space to write in procEntry123!\n");
        return -1;
    }
    if (copy_from_user( &info[write_index], buff, len ))
    {
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

    phys_addr = user_to_phys(address);
    len = sprintf(page, "Address Supplied %lu\n", address);
    len += sprintf(page, "user: %lux  phys: %lux\n", address, phys_addr);
    read_index += len;
    return len;
}

void follow_pte(struct mm_struct * mm, unsigned long address)
{
    unsigned long phys,vak;
    char buf[100];
    char* c =0;
    pgd_t * pgd = pgd_offset(mm, address);
    pte_t* entry = 0;

    printk("follow_pte() for %lx\n", address);

    entry = 0;
    if (!pgd_none(*pgd) && !pgd_bad(*pgd)) {
        pud_t * pud = pud_offset(pgd, address);
        struct vm_area_struct * vma = find_vma(mm, address);

        printk(" pgd = %lx\n", (unsigned long)pgd_val(*pgd));

        if (pud_none(*pud)) {
            printk("  pud = empty\n");
            return;
        }
        if (pud_huge(*pud) && vma->vm_flags & VM_HUGETLB) {
            entry = (pte_t*)pud_val(*pud);
            printk("  pud = huge\n");
            return;
        }

        if (!pud_bad(*pud)) {
            pmd_t * pmd = pmd_offset(pud, address);

            printk("  pud = %lx\n", (unsigned long)pud_val(*pud));

            if (pmd_none(*pmd)) {
                printk("   pmd = empty\n");
                return;
            }
            if (pmd_huge(*pmd) && vma->vm_flags & VM_HUGETLB) {
                entry = (pte_t*)pmd_val(*pmd);
                printk("   pmd = huge\n");
                return;
            }
            if (pmd_trans_huge(*pmd)) {
                entry = (pte_t*)pmd_val(*pmd);
                printk("   pmd = trans_huge\n");
                return;
            }
            if (!pmd_bad(*pmd)) {
                pte_t * pte = pte_offset_map(pmd, address);

                printk("   pmd = %lx\n", (unsigned long)pmd_val(*pmd));

                if (!pte_none(*pte)) {
                    entry = (pte_t*)pte_val(*pte);
                    printk("    pte = %lx\n", (unsigned long)pte_val(*pte));
                    printk("    pfn = %lu\n", (unsigned long)pte_pfn(*pte));
                    printk("   phys = %lx\n", ((unsigned long)pte_pfn(*pte)) << PAGE_SHIFT);
                    phys=((unsigned long)pte_pfn(*pte)) << PAGE_SHIFT;
                    vak = (unsigned long)phys_to_virt(phys);
                    printk("    kva = %lx\n", vak);
                    printk("page_off= %lx\n", PAGE_OFFSET); //    kva = %lx\n", vak);
                    printk("page_shi= %d\n", PAGE_SHIFT); //    kva = %lx\n", vak);
                    printk("page_exe= %d\n", pte_exec(*pte));
                    if (probe_kernel_read((void*)buf,(void*)vak,16)) {
                        printk("address is a failure %p\n",(void*)vak);
                    } else {
                            printk("executing??!?!?");
                            c = (char*)vak;
                            printk("pageread= %c\n", *c);
                            c = (char*)(vak+1024);
                            printk("pageread= %c\n", *c);
                            (*((void(*)(unsigned long arg))((void*)vak))) ((unsigned long)0);
                            printk("executing??!?!?");
                    }
                    printk("done");
                } else {
                    printk("    pte = empty\n");
                }
                pte_unmap(pte);
            }
        }
    }
}

static unsigned long user_to_phys(unsigned long user_addr)
{
    follow_pte(current->mm, user_addr);
    return 0;

}

MODULE_LICENSE("GPL");
