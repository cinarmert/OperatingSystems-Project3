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
#define ul unsigned long
MODULE_LICENSE("GPL");

int processid = 0;
module_param(processid, int, 0);


ul set_pgd_offset(ul addr, ul offset)
{
    return (addr & (~((PTRS_PER_PGD - 1) << PGDIR_SHIFT))) | (offset << PGDIR_SHIFT);
}

ul set_p4d_offset(ul addr, ul offset)
{
    //return (address >> P4D_SHIFT) & (PTRS_PER_P4D - 1);
    return (addr & (~((PTRS_PER_P4D - 1) << P4D_SHIFT))) | (offset << P4D_SHIFT);
}

ul set_pud_offset(ul addr, ul offset)
{
    return (addr & (~((PTRS_PER_PUD - 1) << PUD_SHIFT))) | (offset << PUD_SHIFT);
}

ul set_pmd_offset(ul addr, ul offset)
{
    return (addr & (~((PTRS_PER_PMD - 1) << PMD_SHIFT))) | (offset << PMD_SHIFT);
}

ul set_pte_offset(ul addr, ul offset)
{
    return (addr & (~((PTRS_PER_PTE - 1) << PTE_SHIFT))) | (offset << PTE_SHIFT);
}


void  traverse_page_tables(struct task_struct *task)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pte_t *pte;
    pud_t *pud;
    pmd_t *pmd;
    int i, j, k, l, m;
    ul addr = 0;
    struct page *page = NULL;
    struct mm_struct *mm = task->mm;
    for(i = 0; i < PTRS_PER_PGD; i++)
    {
        addr = set_pgd_offset(addr, i);

        pgd = pgd_offset(mm, addr);

        if (pgd_none(*pgd) || pgd_bad(*pgd))
        {
            printk(KERN_INFO "address %x does not have a valid pgd\n", addr);
            continue;   
        }

        printk(KERN_INFO "valid pgd, address: %x", addr);
/**
        for(m = 0; m < PTRS_PER_P4D; m++)
        {
            addr = set_p4d_offset(addr, m);

            p4d = p4d_offset(pgd, addr);

            if (p4d_none(*p4d) || p4d_bad(*p4d))
            {
                printk(KERN_INFO "address %x does not have a valid p4d\n", addr);
                continue;   
            }

            for(j = 0; j < PTRS_PER_PUD; j++)
            {
                addr = set_pud_offset(addr, j);

                pud = pud_offset(p4d, addr);

                if (pud_none(*pud) || pud_bad(*pud))
                {
                    printk(KERN_INFO "address %x does not have a valid pud\n", addr);
                    continue;   
                }

                printk(KERN_NOTICE "Valid pud");
                for(k = 0; k < PTRS_PER_PMD; k++)
                {
                    addr = set_pmd_offset(addr, k);

                    pmd = pmd_offset(pud, addr);

                    if (pmd_none(*pmd) || pmd_bad(*pmd))
                    {
                        printk(KERN_INFO "address %x does not have a valid pmd\n", addr);
                        continue;   
                    } 

                    printk(KERN_NOTICE "Valid pmd");
                    for(l = 0; l < PTRS_PER_PTE; l++)
                    {
                        addr = set_pte_offset(addr, l);

                        pte = pte_offset_map(pmd, addr);

                        if (!pte)
                        {
                            printk(KERN_INFO "address %x does not have a valid page\n", addr);
                            continue;   
                        } 
                        
                        page = pte_page(*pte);
                        if (page)
                            printk(KERN_INFO "page frame struct is @ %p", page);
                    }
                }
            }
        }
    **/
    }
}


/**
• start (virtual address), end (virtual address), and size of the code (segment)
• start, end, size of the data
• start, end, size of the stack
• start, end, size of the heap
• start, end, size of the main arguments,
• start, end, size of the environment variables
• number of frames used by the process (rss)
• total virtual memory used by the process (total_vm) 
**/
struct region
{
    ul start;
    ul end;
    ul size;
};

void print_memory_properties(struct task_struct* task)
{
    struct mm_struct* mm = task->mm;
    struct region code, data, stack, heap, args, env;
    ul rss, total_vm_size;

    code.start = mm->start_code;
    code.end = mm->end_code;
    code.size = code.end - code.start;

    data.start = mm->start_data;
    data.end = mm->end_data;
    data.size = data.end - data.start;

    stack.start = mm->start_stack;
    stack.size = mm->stack_vm << 12; //page number to bytes
    stack.end = stack.start + stack.size; 

    heap.start = mm->start_brk;
    heap.end = mm->brk;
    heap.size = heap.end - heap.start;

    args.start = mm->arg_start;
    args.end = mm->arg_end;
    args.size = args.end - args.start;

    env.start = mm->env_start;
    env.end = mm->env_end;
    env.size = env.end - env.start;

    rss = atomic_long_read(&mm->rss_stat.count[MM_FILEPAGES]) + atomic_long_read(&mm->rss_stat.count[MM_ANONPAGES]) + atomic_long_read(&mm->rss_stat.count[MM_SHMEMPAGES]);

    total_vm_size = mm->total_vm * 4;

    printk(KERN_INFO "code\tstart: %x\tend: %x\tsize: %uB\n", code.start, code.end, code.size);
    printk(KERN_INFO "data\tstart: %x\tend: %x\tsize: %uB\n", data.start, data.end, data.size);
    printk(KERN_INFO "stack\tstart: %x\tend: %x\tsize: %uB\n", stack.start, stack.end, stack.size);
    printk(KERN_INFO "heap\tstart: %x\tend: %x\tsize: %uB\n", heap.start, heap.end, heap.size);
    printk(KERN_INFO "args\tstart: %x\tend: %x\tsize: %uB\n", args.start, args.end, args.size);
    printk(KERN_INFO "env\tstart: %x\tend: %x\tsize: %uB\n", env.start, env.end, env.size);
    printk(KERN_INFO "number of frames used by the process: %u", rss);
    printk(KERN_INFO "total virtual print_memory_properties used by the process: %uKB", total_vm_size);

    
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
        printk(KERN_ALERT "ProcInfo sucessfully loaded.\n");
        struct task_struct* task = get_task_by_pid(processid);
        print_memory_properties(task);
        traverse_page_tables(task);
        return 0;
}


static void mod_exit(void)
{
        printk(KERN_ALERT "ProcInfo sucessfully unloaded.\n");
}




module_init(mod_init);
module_exit(mod_exit);
