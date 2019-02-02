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
    stack.size = mm->stack_vm << PAGE_SHIFT; //page number to bytes
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

    printk(KERN_INFO "code\tstart: %llx\tend: %llx\tsize: %uB\n", code.start, code.end, code.size);
    printk(KERN_INFO "data\tstart: %llx\tend: %llx\tsize: %uB\n", data.start, data.end, data.size);
    printk(KERN_INFO "stack\tstart: %llx\tend: %llx\tsize: %uB\n", stack.start, stack.end, stack.size);
    printk(KERN_INFO "heap\tstart: %llx\tend: %llx\tsize: %uB\n", heap.start, heap.end, heap.size);
    printk(KERN_INFO "args\tstart: %llx\tend: %llx\tsize: %uB\n", args.start, args.end, args.size);
    printk(KERN_INFO "env\tstart: %llx\tend: %llx\tsize: %uB\n", env.start, env.end, env.size);
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

        printk(KERN_ALERT "module sucessfully loaded.\n");
        struct task_struct* task = get_task_by_pid(processid);
        print_memory_properties(task);
        return 0;
}


static void mod_exit(void)
{

        printk(KERN_ALERT "module sucessfully unloaded.\n");
}




module_init(mod_init);
module_exit(mod_exit);
