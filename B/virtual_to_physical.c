#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/proc_fs.h>
#include <asm/pgtable.h>

#define ul unsigned long long
MODULE_LICENSE("GPL");

int processid = 0;
long virtaddr = 0;
module_param(processid, int, 0);
module_param(virtaddr, long, 0);


void  traverse_page_tables(struct task_struct *task, ul virtaddr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pte_t *pte;
    pud_t *pud;
    pmd_t *pmd;
    ul offset;
    ul frame_number;
    ul addr = virtaddr;
    ul physical_address;
    struct mm_struct *mm = task->mm;

    offset = ((addr >> PAGE_SHIFT) << PAGE_SHIFT) ^ addr;
    pgd = pgd_offset(mm, addr);


    if (pgd_none(*pgd) || pgd_bad(*pgd) || !pgd_present(*pgd))
    {
        printk(KERN_NOTICE "address %llx does not have a valid pgd\n", addr);
        return;
    }

    p4d = p4d_offset(pgd, addr);

    if (p4d_none(*p4d) || p4d_bad(*p4d) || !p4d_present(*p4d))
    {
        printk(KERN_NOTICE "address %llx does not have a valid p4d\n", addr);
        return;   
    }

    pud = pud_offset(p4d, addr);

    if (pud_none(*pud) || pud_bad(*pud) || !pud_present(*pud))
    {
        printk(KERN_NOTICE "address %llx does not have a valid pud\n", addr);
        return;   
    }

    pmd = pmd_offset(pud, addr);

    if (pmd_none(*pmd) || pmd_bad(*pmd) || !pmd_present(*pmd))
    {
        printk(KERN_NOTICE "address %llx does not have a valid pmd\n", addr);
        return;   
    } 

    pte = pte_offset_map(pmd, addr);

    if (pte_none(*pte) || !pte_present(*pte))
    {
        printk(KERN_NOTICE "address %llx does not have a valid page\n", addr);
        return;   
    } 

    frame_number = pte_pfn(*pte);
    physical_address = (frame_number << PAGE_SHIFT) + offset;

    printk(KERN_INFO "virtual address: %llx, corresponds to physical address %llx\n", virtaddr, physical_address);

}




struct task_struct* get_task_by_pid(unsigned int pid)
{
    struct task_struct *task;
    rcu_read_lock();
    int found = 0;
    for_each_process(task)
    {
        task_lock(task); 
        if(task->pid == pid)
            found = 1;
        task_unlock(task);
        if(found)
            break;
    }
    rcu_read_unlock(); 
    return task;
}

static int mod_init(void)
{

        printk(KERN_ALERT "module sucessfully loaded.\n");
        struct task_struct* task = get_task_by_pid(processid);
        traverse_page_tables(task, virtaddr);
        return 0;
}


static void mod_exit(void)
{

        printk(KERN_ALERT "module sucessfully unloaded.\n");
}




module_init(mod_init);
module_exit(mod_exit);
