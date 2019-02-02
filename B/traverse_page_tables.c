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
module_param(processid, int, 0);


ul set_pgd_offset(ul addr, ul offset)
{
    return (addr & ((~(PTRS_PER_PGD - 1) << PGDIR_SHIFT))) | (offset << PGDIR_SHIFT);
}

ul set_p4d_offset(ul addr, ul offset)
{
    //return (address >> P4D_SHIFT) & (PTRS_PER_P4D - 1);
    return (addr & ((~(PTRS_PER_P4D - 1) << P4D_SHIFT))) | (offset << P4D_SHIFT);
}

ul set_pud_offset(ul addr, ul offset)
{
    return (addr & ((~(PTRS_PER_PUD - 1) << PUD_SHIFT))) | (offset << PUD_SHIFT);
}

ul set_pmd_offset(ul addr, ul offset)
{
    return (addr & ((~(PTRS_PER_PMD - 1) << PMD_SHIFT))) | (offset << PMD_SHIFT);
}

ul set_pte_offset(ul addr, ul offset)
{
    return (addr & ((~(PTRS_PER_PTE - 1) << PTE_SHIFT))) | (offset << PTE_SHIFT);
}


void  traverse_page_tables(struct task_struct *task)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pte_t *pte;
    pud_t *pud;
    pmd_t *pmd;
    ul i, j, k, l, m;
    ul addr = 0;
    struct page *page = NULL;
    struct mm_struct *mm = task->mm;
    for(i = 0; i < PTRS_PER_PGD; i++)
    {
        addr = set_pgd_offset(addr, i);

        pgd = pgd_offset(mm, addr);

        if (pgd_none(*pgd) || pgd_bad(*pgd) || !pgd_present(*pgd))
        {
            printk(KERN_NOTICE "address %llx does not have a valid pgd\n", addr);
            continue;   
        }

        printk(KERN_INFO "valid pgd, address: %llx, frame number: %llx\n", addr, pgd_pfn(*pgd));
        for(m = 0; m < PTRS_PER_P4D; m++)
        {
            addr = set_p4d_offset(addr, m);

            p4d = p4d_offset(pgd, addr);

            if (p4d_none(*p4d) || p4d_bad(*p4d) || !p4d_present(*p4d))
            {
                printk(KERN_NOTICE "address %llx does not have a valid p4d\n", addr);
                continue;   
            }
            printk(KERN_INFO "valid p4d, address: %llx, frame number: %llx\n", addr, p4d_pfn(*p4d));

            for(j = 0; j < PTRS_PER_PUD; j++)
            {
                addr = set_pud_offset(addr, j);

                pud = pud_offset(p4d, addr);

                if (pud_none(*pud) || pud_bad(*pud) || !pud_present(*pud))
                {
                    printk(KERN_NOTICE "address %llx does not have a valid pud\n", addr);
                    continue;   
                }

                printk(KERN_INFO "valid pud, address: %llx, frame number: %llx\n", addr, pud_pfn(*pud));
                for(k = 0; k < PTRS_PER_PMD; k++)
                {
                    addr = set_pmd_offset(addr, k);

                    pmd = pmd_offset(pud, addr);

                    if (pmd_none(*pmd) || pmd_bad(*pmd) || !pmd_present(*pmd))
                    {
                        printk(KERN_NOTICE "address %llx does not have a valid pmd\n", addr);
                        continue;   
                    } 

                    printk(KERN_INFO "valid pmd, address: %llx, frame number: %llx\n", addr, pmd_pfn(*pmd));
                    for(l = 0; l < PTRS_PER_PTE; l++)
                    {
                        addr = set_pte_offset(addr, l);

                        pte = pte_offset_map(pmd, addr);

                        if (pte_none(*pte) || !pte_present(*pte))
                        {
                            printk(KERN_NOTICE "address %llx does not have a valid page\n", addr);
                            continue;   
                        } 
                        
                        page = pte_page(*pte);
                        if (page)
                            printk(KERN_INFO "address %llx has a valid page, frame number: %llx\n", addr, pte_pfn(*pte));
                    }
                }
            }
        }
    }
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
        traverse_page_tables(task);
        return 0;
}


static void mod_exit(void)
{

        printk(KERN_ALERT "module sucessfully unloaded.\n");
}




module_init(mod_init);
module_exit(mod_exit);
