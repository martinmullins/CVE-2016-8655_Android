#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>

#define MAX_LEN       4096
int read_info( char *page, char **start, off_t off,int count, int *eof, void *data );
ssize_t write_info( struct file *fopy of lookup_address_in_pgd() from arch/x86/mm/pageattr.c
 */
static pte_t *__lookup_addr_in_pgd(pgd_t *pgd, unsigned long addr,
                                   unsigned int *level)
{
        pud_t *pud;
        pmd_t *pmd;

        *level = PG_LEVEL_NONE;

        if (pgd_none(*pgd))
                return NULL;

        pud = pud_offset(pgd, addr);
        if (pud_none(*pud))
                return NULL;

        *level = PG_LEVEL_1G;
        if (pud_large(*pud) || !pud_present(*pud))
                return (pte_t *)pud;

        pmd = pmd_offset(pud, addr);
        if (pmd_none(*pmd))
                return NULL;

        *level = PG_LEVEL_2M;
        if (pmd_large(*pmd) || !pmd_present(*pmd))
                return (pte_t *)pmd;

        *level = PG_LEVEL_4K;

        return pte_offset_kernel(pmd, addr);
}

/*
 * Look up a virtual (process or kernel) address and return its PTE
 *
 * Based on lookup_address() from arch/x86/mm/pageattr.c
 */
static pte_t *__lookup_addr(unsigned long addr, unsigned int *level)
{
        pgd_t *pgd;

        if (addr > PAGE_OFFSET) {
                /* kernel virtual address */
                pgd = pgd_offset(__init_mm, addr);
        } else {
                /* user (process) virtual address */
                pgd = pgd_offset(current->mm, addr);
        }

        return __lookup_addr_in_pgd(pgd, addr, level);
}

/*
 * Convert a user (process) virtual address to a physical address
 *
 * Based on slow_virt_to_phys() from arch/x86/mm/pageattr.c
 */
static unsigned long user_to_phys(unsigned long user_addr)
{
        unsigned long phys_addr;
        unsigned long offset;
        unsigned int level;
        pte_t *pte;

        printk("ret2usr: pid: %d, comm: %s\n", current->pid, current->comm);

        pte = __lookup_addr(user_addr, &level);
        if (!pte)
                return 0;

        /*
         * pXX_pfn() returns unsigned long, which must be cast to phys_addr_t
         * before being left-shifted PAGE_SHIFT bits -- this trick is to
         * make 32-PAE kernel work correctly.
         */
        switch (level) {
        case PG_LEVEL_1G:
                phys_addr = (unsigned long)pud_pfn(*(pud_t *)pte) << PAGE_SHIFT;
                offset = user_addr & ~PUD_PAGE_MASK;
                break;
        case PG_LEVEL_2M:
                phys_addr = (unsigned long)pmd_pfn(*(pmd_t *)pte) << PAGE_SHIFT;
                offset = user_addr & ~PMD_PAGE_MASK;
                break;
        default:
                phys_addr = (unsigned long)pte_pfn(*pte) << PAGE_SHIFT;
                offset = user_addr & ~PAGE_MASK;
        }

        return (unsigned long)(phys_addr | offset);
}


static struct proc_dir_entry *proc_entry;
static char *info;
static int write_index;
static int read_index;
static unsigned long int address;

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
    if (len > capacity)
    {
        printk(KERN_INFO "No space to write in procEntry123!\n");
        return -1;
    }
    if (copy_from_user( &info[write_index], buff, len ))
    {
        return -2;
    }

    write_index += len;
    info[write_index-1] = 0;
    //printk("This is a number %lu\n",simple_strtol(info,&endp,10));
    address = simple_strtol(info,&endp,10);
    return len;
}

int read_info( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    int len;
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    if (read_index >= write_index)
    read_index = 0;

    //len = sprintf(page, "%s\n", &info[read_index]);
    len = sprintf(page, "Address Supplied %lu\n", address);
    read_index += len;
    return len;
}
MODULE_LICENSE("GPL");
