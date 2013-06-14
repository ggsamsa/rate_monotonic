/*
 *  linux/fs/proc/kmsg.c
 *
 *  Copyright (C) 1992  by Linus Torvalds
 *
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/syslog.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#ifdef CONFIG_SCHED_CASIO_POLICY
	#include <linux/sched_casio.h>
#endif

extern wait_queue_head_t log_wait;

static int kmsg_open(struct inode * inode, struct file * file)
{
	return do_syslog(SYSLOG_ACTION_OPEN, NULL, 0, SYSLOG_FROM_FILE);
}

static int kmsg_release(struct inode * inode, struct file * file)
{
	(void) do_syslog(SYSLOG_ACTION_CLOSE, NULL, 0, SYSLOG_FROM_FILE);
	return 0;
}

static ssize_t kmsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	if ((file->f_flags & O_NONBLOCK) &&
	    !do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_FILE))
		return -EAGAIN;
	return do_syslog(SYSLOG_ACTION_READ, buf, count, SYSLOG_FROM_FILE);
}

static unsigned int kmsg_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &log_wait, wait);
	if (do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_FILE))
		return POLLIN | POLLRDNORM;
	return 0;
}


static const struct file_operations proc_kmsg_operations = {
	.read		= kmsg_read,
	.poll		= kmsg_poll,
	.open		= kmsg_open,
	.release	= kmsg_release,
};

#ifdef CONFIG_SCHED_CASIO_POLICY
static int casio_trace_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "SCHED_INFO:0: _casio_trace_open\n");
	filp->private_data = get_casio_trace_log();
	if(!filp->private_data) return -1;
	return 0;
}

static ssize_t casio_trace_read(struct file *filp, char __user *buf, size_t nbytes, loff_t *ppos)
{
		int ret = 0;
		unsigned long len = 0;
		unsigned int nitems = 0, front = 0;
		char buffer[100];
		struct casio_trace_event_log *log = (struct casio_trace_event_log*)filp->private_data;
		buffer[0] = 0;
		nitems = atomic_read(&log->nitems);
		if(nitems > 0)
		{
				front = atomic_read(&log->front);
				len = snprintf(buffer, nbytes, "%d, %llu, %d, %d, %d\n",
				log->casio_trace_event[front].event, 
				log->casio_trace_event[front].time, 
				log->casio_trace_event[front].casio_id, 
				log->casio_trace_event[front].job_nr,
				log->casio_trace_event[front].pid);
				if(len > nbytes)
				{
					printk(KERN_INFO "SCHED_CASIO:error:nbytes:%d:len:%ld", nbytes, len);
					return -EINVAL;
				}
				if((ret=copy_to_user(buf, buffer, len)))
				{
						printk(KERN_INFO "SCHED_CASIO:error:copy_to_user:%ld:%d:%s", len, ret, buffer);
						return -EFAULT;
				}
				
				front = (front + 1 >= CASIO_TRACE_EVENT_SIZE)?0:front+1;
				atomic_set(&log->front, front); atomic_dec(&log->nitems);
				return (ssize_t)len;
		}
		return 0;
}
static int casio_trace_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "SCHED_CASIO:0: _casio_trace_release\n");
	filp->private_data = NULL;
	return 0;	
}

static const struct file_operations proc_casio_trace_operations = {
	.open = casio_trace_open,
	.read = casio_trace_read,
	.release = casio_trace_release
};
#endif

static int __init proc_kmsg_init(void)
{
	proc_create("kmsg", S_IRUSR, NULL, &proc_kmsg_operations);
#ifdef CONFIG_SCHED_CASIO_POLICY
	proc_create("casio_trace", 0444, NULL, &proc_casio_trace_operations);
#endif
	return 0;
}
module_init(proc_kmsg_init);
