/*
casio-task scheduling class
*/
#include <linux/sched_casio.h>
#include "sched_casio_release.c"
#include "sched_casio_trace.c"


asmlinkage long sys_casio_clock(char __user *t)
{
	unsigned long long now = casio_clock();
	if(copy_to_user(t, &now, sizeof(unsigned long long)))
		return -EFAULT;
	return 0;
}

asmlinkage long sys_casio_sched_setscheduler(unsigned int task_id, unsigned long h_deadline, unsigned long l_deadline, unsigned long priority)
{
	struct sched_param param;
	param.sched_priority = 1;
	
	current->casio_task.id = task_id;
	current->casio_task.deadline = 0;
	current->casio_task.deadline = h_deadline;
	current->casio_task.deadline <<= 32;
	current->casio_task.deadline |= l_deadline;
	current->casio_task.priority = priority;

	current->casio_task.job.priority = priority;
	current->casio_task.job.deadline = 0;
	current->casio_task.job.release = 0;
	atomic_set(&current->casio_task.job.nr, 0);

	return sched_setscheduler(current, SCHED_CASIO, &param);
}

/*
enqueue a task into the ready queue (a binary tree)
rms points to the task with earliest deadline
*/
void _enqueue_task_casio(struct rq *rq, struct task_struct *p)
{
	struct rb_node **node;
	struct rb_node *parent = NULL;
	struct casio_task *entry;
	int rms = 1;
	node = &rq->casio_rq.ready.tasks.rb_node;
	while(*node!=NULL){
		parent = *node;
		entry = rb_entry(parent, struct casio_task, ready_node);
		if(p->casio_task.job.deadline < entry->job.deadline)
		{
			node = &parent->rb_left;
		}
		else
		{
			node = &parent->rb_right;
			rms=0;
		}
	}
	if(rms)
	{
		rq->casio_rq.ready.rms = p;
	}

	rb_link_node(&p->casio_task.ready_node, parent, node);
	rb_insert_color(&p->casio_task.ready_node, &rq->casio_rq.ready.tasks);
}

/*dequeue a task from the ready queue (a binary tree)
rms points to the task with earliest deadline; if there is no task it is NULL
*/
void _dequeue_task_casio(struct rq *rq, struct task_struct *p)
{
	struct rb_node *node;
	struct casio_task *entry;
	if(rq->casio_rq.ready.rms == p)
	{
		rq->casio_rq.ready.rms = NULL;
		node = rb_next(&p->casio_task.ready_node);
		if(node)
		{
			entry = rb_entry(node, struct casio_task, ready_node);
			rq->casio_rq.ready.rms = (struct task_struct*) container_of(entry, struct task_struct, casio_task);
		}
	}
	rb_erase(&p->casio_task.ready_node, &rq->casio_rq.ready.tasks);
	p->casio_task.ready_node.rb_left = p->casio_task.ready_node.rb_right = NULL;
}

static void
enqueue_task_casio(struct rq *rq, struct task_struct *p, int wakeup)
{
	unsigned long long now = casio_clock();
	p->casio_task.job.deadline = now + p-> casio_task.deadline;
	_enqueue_task_casio(rq, p);
	casio_trace(ENQUEUE, now, p);
}

static void dequeue_task_casio(struct rq *rq, struct task_struct *p, int sleep)
{
	_dequeue_task_casio(rq, p);
	casio_trace(DEQUEUE, casio_clock(), p);
}

static void
yield_task_casio(struct rq *rq)
{
	return;
}

static void
check_preempt_curr_casio(struct rq *rq, struct task_struct *p, int flags)
{
	if(rq->casio_rq.ready.rms!=NULL)
	{
		if(rq->casio_rq.ready.rms != rq->curr)
			resched_task(rq->curr);

	}
}

static struct task_stuct *
pick_next_task_casio(struct rq *rq)
{
	return rq->casio_rq.ready.rms;
}

static void
put_prev_task_casio(struct rq *rq, struct task_struct *prev)
{
	return;
}

static void 
task_tick_casio(struct rq *rq, struct task_struct *curr, int queued)
{
	check_preempt_curr_casio(rq, curr, 1);
}

#ifdef CONFIG_SMP
static int select_task_rq_casio(struct task_struct *p, int sd_flag, int flags)
{
	return task_cpu(p);
}
#endif

static void
set_curr_task_casio(struct rq *rq)
{
	return;
}

static void
switched_to_casio(struct rq *rq, struct task_struct *p, int running)
{
	return;
}

static void
prio_changed_casio(struct rq *rq, struct task_struct *p, int oldprio, int running)
{
	return;
}

static unsigned int
get_rr_interval_casio(struct rq *rq, struct task_struct *task)
{
	return 0;
}

static const struct sched_class casio_sched_class = {
        .next           = &rt_sched_class,
        .enqueue_task   = enqueue_task_casio,
        .dequeue_task   = dequeue_task_casio,
	.yield_task	= yield_task_casio,
        .check_preempt_curr = check_preempt_curr_casio,
        .pick_next_task = pick_next_task_casio,
	.put_prev_task	= put_prev_task_casio,

#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_casio,
#endif
	.set_curr_task	= set_curr_task_casio,
	.task_tick	= task_tick_casio,
	.get_rr_interval = get_rr_interval_casio,
	.prio_changed	= prio_changed_casio,
	.switched_to	= switched_to_casio,
};


void init_casio_task_ready(struct casio_task_ready *r)
{
	r->tasks = RB_ROOT;
	r->rms = NULL;
}
void init_casio_rq(struct casio_rq *casio_rq)
{
	printk(KERN_INFO "SCHED_CASIO: init_casio_rq\n");
	init_casio_task_ready(&casio_rq->ready);
	init_casio_task_release(&casio_rq->release);
}
/*
static struct task_struct * pick_next_task_casio(struct rq *rq)
{
	return rq->casio_rq.ready.rms;
}*/
