#include <linux/kernel.h>
#include <linux/sched.h>

asmlinkage int sys_printself(void) {
	/*
 * 	struct task_struct* task = current;

	printk(KERN_EMERG "Process ID: %lu\n", task->pid);
	printk(KERN_EMERG "State: %ld\n", task->state);
	printk(KERN_EMERG "Name: %s\n", task->comm);
	printk(KERN_EMERG "User Time: %lu\n", task->vruntime);
	printk(KERN_EMERG "System Time: %lu\n", task->stime);
	
	for(;task != &init_task; task = task->parent) {
		printk(KERN_EMERG "Parent PID: %lu     Parent Name: %s\n",task->pid, task->comm);
	}	
	return 0;
	*/
}
