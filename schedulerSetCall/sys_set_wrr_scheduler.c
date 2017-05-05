#include <sched.h>

asmlinkage int sys_set_wrr_scheduler(int pid, int weight) {
	const struct sched_param* param = kmalloc(sizeof(struct sched_param), GFP_ATOMIC);
	param->sched_priority = weight;
	sched_setscheduler(pid, 6, param);
	return 0;
}
