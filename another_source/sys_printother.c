#include <linux/kernel.h>
#include <linux/sched.h>

asmlinkage int sys_printother(int arb_pid) {
	struct task_struct* task = current;
	for(;task != &init_task; task = task->parent) {
		if (task->pid == arb_pid) {
			printk(KERN_EMERG "TASK FOUND!");
			break;
		}
	}
	printk(KERN_EMERG "Process ID: %lu\n", task->pid);
	printk(KERN_EMERG "Process Name: %s\n", task->comm);
	printk(KERN_EMERG "Process UTime: %lu\n", task->utime);
	printk(KERN_EMERG "Process STime: %lu\n", task->stime);
	return 0;
}

