/*
This is a global variable and it is implement for multiprocessor systems
*/

struct casio_trace_event_log casio_trace_event_log[NR_CPUS];

/*
This function registers the event
*/

void casio_trace(enum casio_event event, unsigned long long time, struct task_struct *p)
{
	int nitems = 0;
	int rear = 0;
	struct casio_trace_event_log *log = get_casio_trace_log();
	if(log)
	{
		nitems = atomic_read(&log->nitems);
		if(nitems < CASIO_TRACE_EVENT_SIZE)
		{
			rear = atomic_read(&log->rear);
			log->casio_trace_event[rear].event = event;
			log->casio_trace_event[rear].time = time;
			if(p->policy == SCHED_CASIO)
			{
				log->casio_trace_event[rear].casio_id = p->casio_task.id;
				log->casio_trace_event[rear].job_nr = atomic_read(&p->casio_task.job.nr);
				log->casio_trace_event[rear].deadline = p->casio_task.job.deadline;
			}
			else
			{
				log->casio_trace_event[rear].casio_id = 0;
				log->casio_trace_event[rear].job_nr = 0;
				log->casio_trace_event[rear].deadline = 0;
			}
			log->casio_trace_event[rear].state = p->state;
			log->casio_trace_event[rear].pid = p->pid;
			atomic_inc(&log->rear);
			rear = (rear+1 >= CASIO_TRACE_EVENT_SIZE) ?0:rear+1;
			atomic_set(&log->rear, rear);
			atomic_inc(&log->nitems);
		}
	}
}

struct casio_trace_event_log * get_casio_trace_log(void)
{
	struct casio_trace_event_log *l = &casio_trace_event_log[smp_processor_id()];
	if(!l)
		printk(KERN_INFO "SCHED_CASIO:get_casio_trace_log\n");
	return l;
}

asmlinkage long sys_casio_trace_init(void)
{
	struct casio_trace_event_log *log = get_casio_trace_log();
	atomic_set(&log->front, 0);
	atomic_set(&log->rear, 0);
	atomic_set(&log->nitems, 0);
	return 0;
}
