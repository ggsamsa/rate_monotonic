#ifndef __SCHED_CASIO_H_
#define __SCHED_CASIO_H_

#include <linux/sched.h>



#define ns_to_ktime(t) ktime_add_ns (ktime_set (0,0), t)

static inline unsigned long long casio_clock (void)
{
	return ktime_to_ns (ktime_get());
}

/**********************************************/
/*********** TRACING **************************/
/*
  This data structures and function are used by the trace system.
  Trace system is coded into two files: kernel/casio/trace.c and fs/proc/kmsg.c
  Trace system registers some events into a circular queue.
*/

enum casio_event
{
	ENQUEUE = 0,
	DEQUEUE,
	SWITCH_TO,
	SWITCH_AWAY,
	SWITCH_TO_CASIO,
	ENQUEUE_RELEASE,
	DEQUEUE_RELEASE,
};

struct casio_trace_event
{
	enum casio_event event;
	unsigned long long time;
	unsigned int casio_id;
	unsigned int job_nr;
	pid_t pid;
	unsigned int state;
	unsigned long long deadline;
};

#define CASIO_TRACE_EVENT_SIZE 10000

struct casio_trace_event_log
{
	struct casio_trace_event casio_trace_event[CASIO_TRACE_EVENT_SIZE];
	atomic_t front;
	atomic_t rear;
	atomic_t nitems;
};

void casio_trace(enum casio_event event, unsigned long long time, struct task_struct *p);
struct casio_trace_event_log *get_casio_trace_log(void);
#define EPSILON 10000
void casio_setup_release_timer(void);
#endif
