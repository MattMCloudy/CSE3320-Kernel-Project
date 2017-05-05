/*
 * Weighted Round Robin Scheduling Class (mapped to the SCHED_WRR
 * policy)
 */

#include <linux/slab.h>
#include <linux/list.h>
#include <linux/types.h>


#define for_each_sched_wrr_entity(wrr_se) \
	for (; wrr_se; wrr_se = wrr_se->parent)


static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
	return container_of(wrr_se, struct task_struct, wrr);
}

static inline u64 sched_wrr_runtime(struct wrr_rq *wrr_rq)
{
	if (!wrr_rq->tg)
		return RUNTIME_INF;

	return wrr_rq->wrr_runtime;
}

static inline struct wrr_rq *wrr_rq_of_se(struct sched_wrr_entity *wrr_se)
{
	struct task_struct *p = wrr_task_of(wrr_se);
	struct rq *rq = task_rq(p);

	return &rq->wrr;
}

static void update_curr_wrr(struct rq* rq) {
	
	struct task_struct *curr = rq->curr;
	struct sched_wrr_entity *wrr_se = &curr->wrr;
	struct wrr_rq *wrr_rq = wrr_rq_of_se(wrr_se);
	u64 delta_exec;

	if (curr != 6)
		return;

	delta_exec = rq->clock - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = rq->clock;
	cpuacct_charge(curr, delta_exec);

}

static inline int on_wrr_rq(struct sched_wrr_entity *wrr_se)
{
	return !list_empty(&wrr_se->run_list);
}

static void dequeue_wrr_stack(struct sched_wrr_entity *wrr_se)
{
	struct sched_wrr_entity *back = NULL;

	for_each_sched_wrr_entity(wrr_se) {
		wrr_se->back = back;
		back = wrr_se;
	}

	for (wrr_se = back; wrr_se; wrr_se = wrr_se->back) {
		if (on_wrr_rq(wrr_se))
			__dequeue_rt_entity(rt_se);
	}
}

static void enqueue_pushable_task(struct rq *rq, struct task_struct *p)
{
	plist_del(&p->pushable_tasks, &rq->wrr.pushable_tasks);
	plist_node_init(&p->pushable_tasks, p->prio);
	plist_add(&p->pushable_tasks, &rq->wrr.pushable_tasks);
}

static void dequeue_pushable_task(struct rq *rq, struct task_struct *p)
{
	plist_del(&p->pushable_tasks, &rq->wrr.pushable_tasks);
}


static void enqueue_wrr_entity(struct sched_wrr_entity *wrr_se)
{
	dequeue_wrr_stack(wrr_se);
	for_each_sched_wrr_entity(wrr_se)
		__enqueue_wrr_entity(wrr_se);
}

static void dequeue_wrr_entity(struct sched_wrr_entity *wrr_se)
{
	dequeue_wrr_stack(wrr_se);

	for_each_sched_wrr_entity(wrr_se) {
		struct wrr_rq *wrr_rq = NULL;

		if (wrr_rq && wrr_rq->wrr_nr_running)
			__enqueue_wrr_entity(wrr_se);
	}
}

static inline int has_pushable_tasks(struct rq *rq)
{
	return !plist_head_empty(&rq->rt.pushable_tasks);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int wakeup)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	if (wakeup)
		wrr_se->timeout = 0;

	enqueue_wrr_entity(wrr_se);

	if (!task_current(rq, p) && p->wrr.nr_cpus_allowed > 1)
		enqueue_pushable_task(rq, p);
}
	 

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int sleep)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	update_curr_wrr(rq);
	dequeue_wrr_entity(wrr_se);

	dequeue_pushable_task(rq, p);
}

static void requeue_wrr_entity(struct wrr_rq *wrr_rq, struct sched_wrr_entity* wrr_se, int head)
{	

}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{	

	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq;

	for_each_sched_wrr_entity(wrr_se) {
		wrr_rq = wrr_rq_of_se(wrr_se);
			requeue_wrr_entity(wrr_rq, wrr_se, head);
		}
}

static void yield_task_wrr(struct rq *rq)
{

}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static struct sched_wrr_entity *pick_next_wrr_entity(struct rq *rq,
						   struct wrr_rq *wrr_rq)
{
	struct wrr_prio_array *array = &wrr_rq->active;
	struct sched_wrr_entity *next = NULL;
	struct list_head *queue;
	int idx;

	idx = sched_find_first_bit(array->bitmap);
	BUG_ON(idx >= MAX_WRR_PRIO);

	queue = array->queue + idx;
	next = list_entry(queue->next, struct sched_wrr_entity, run_list);

	return next;
}

static struct task_struct *_pick_next_task_wrr(struct rq *rq)
{
	struct sched_wrr_entity *wrr_se;
	struct task_struct *p;
	struct wrr_rq *wrr_rq;

	wrr_rq = &rq->wrr;

	if (unlikely(!wrr_rq->wrr_nr_running))
		return NULL;

	if (wrr_rq_throttled(wrr_rq))
		return NULL;

	do {
		wrr_se = pick_next_wrr_entity(rq, wrr_rq);
		BUG_ON(!wrr_se);
		wrr_rq = group_wrr_rq(wrr_se);
	} while (wrr_rq);

	p = wrr_task_of(wrr_se);
	p->se.exec_start = rq->clock;

	return p;
}

static struct task_struct *pick_next_task_wrr(struct rq *rq)
{
	struct task_struct *p = _pick_next_task_wrr(rq);

	/* The running task is never eligible for pushing */
	if (p)
		dequeue_pushable_task(rq, p);

	/*
	 * We detect this state here so that we can avoid taking the RQ
	 * lock again later if there is no need to push
	 */
	rq->post_schedule = has_pushable_tasks(rq);

	return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{

	update_curr_wrr(rq);
	p->se.exec_start = 0;

	/*
	 * The previous task needs to be made eligible for pushing
	 * if it is still active
	 */
	if (p->se.on_rq && p->wrr.nr_cpus_allowed > 1)
		enqueue_pushable_task(rq, p);
}

#ifdef CONFIG_SMP
static int select_task_rq_wrr(struct task_struct *p, int sd_flag, int flags)
{
	return task_cpu(p);
}

static unsigned long
load_balance_wrr(struct rq *this_rq, int this_cpu, struct rq *busiest,
                unsigned long max_load_move,
                struct sched_domain *sd, enum cpu_idle_type idle,
                int *all_pinned, int *this_best_prio)
{
        /* don't touch WRR tasks */
        return 0;
}

static int
move_one_task_wrr(struct rq *this_rq, int this_cpu, struct rq *busiest,
                 struct sched_domain *sd, enum cpu_idle_type idle)
{
        return 0;
}


#endif
static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;

	p->se.exec_start = rq->clock;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_wrr(rq);

	watchdog(rq, p);

	/*
	 * RR tasks need a special form of timeslice management.
	 * FIFO tasks have no timeslices.
	 */
	if (p->policy != SCHED_RR)
		return;

	if (--p->wrr.time_slice)
		return;

	p->wrr.time_slice = DEF_TIMESLICE;

	/*
	 * Requeue to the end of queue if we are not the only element
	 * on the queue:
	 */
	if (p->wrr.run_list.prev != p->wrr.run_list.next) {
		requeue_task_wrr(rq, p, 0);
		set_tsk_need_resched(p);
	}
}

unsigned int get_rr_interval_wrr(struct task_struct *task)
{
        /*
 *          * Time slice is 0 for SCHED_FIFO tasks
 *                   */
        if (task->policy == SCHED_WRR)
                return DEF_TIMESLICE;
        else
                return 0;
}
/* added by Jia Rao: No preemption, so we leave this function empty */
static void prio_changed_wrr(struct rq *rq, struct task_struct *p,
                              int oldprio, int running)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p,
                           int running)
{
}

static const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,

	.load_balance		= load_balance_wrr,
	.move_one_task		= move_one_task_wrr,
#endif

	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

//	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};

