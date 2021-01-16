#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROCFS_MAX_SIZE 100;
#define MONITORING_NODENAME "rt_monitor"
//==============================================================================
//==============================================================================
struct custom_task_struct_t{
  // Scheduling infos
  /* # of times we have run on this CPU: */
	unsigned long			pcount;

	/* Time spent waiting on a runqueue: */
	unsigned long long		run_delay;

	/* Timestamps: */

	/* When did we last run on a CPU? */
	unsigned long long		last_arrival;

	/* When were we last queued to run? */
	unsigned long long		last_queued;

  uint64_t utime;
  uint64_t stime;
  /* Context switch counts: */
  unsigned long			nvcsw;
	unsigned long			nivcsw;

};
static struct proc_dir_entry *ent;
//==============================================================================
//==============================================================================
int __get_children(struct task_struct** children){
  struct list_head *list;

  list_for_each(list, &current->children) {
    if(!*children){
      *children = list_entry( list, struct task_struct, sibling );
    }
    else{
      printk(KERN_ERR "Too much children for this process\n");
      return -1;
    }
  }

  if(*children)
    return 0;

  return -2;
}
//==============================================================================
static ssize_t send_child_vitals(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
  struct task_struct* children = NULL;
  struct custom_task_struct_t vitals;
  //struct sched_info sched_info;

  /* get buffer size */
	if ( count != sizeof(struct custom_task_struct_t) ) {
		printk(KERN_ERR "The requested size is different from the {struct custom_task_struct_t} size\n");
    return -EFAULT;
	}

  if(__get_children(&children)){
    printk(KERN_ERR "Leaving the kernel module with error\n");
    return -1;
  }

  vitals.pcount = children->sched_info.pcount;
  vitals.run_delay = children->sched_info.run_delay;
  vitals.last_arrival = children->sched_info.last_arrival;
  vitals.last_queued = children->sched_info.last_queued;

  vitals.utime = children->utime;
  vitals.stime = children->stime;

  vitals.nvcsw = children->nvcsw;
  vitals.nivcsw = children->nivcsw;

	/* write data to the buffer */
  if( copy_to_user( ubuf, &vitals, count ) ){
     return -EFAULT;
  }

	return count;
}
//==============================================================================
//==============================================================================
static struct proc_ops myops={
    .proc_read = send_child_vitals
    //.proc_write = mywrite
};
//==============================================================================
//==============================================================================
static int simple_init(void)
{
	ent=proc_create(MONITORING_NODENAME,0444,NULL,&myops);
	return 0;
}
//==============================================================================
//==============================================================================
static void simple_cleanup(void)
{
	proc_remove(ent);
}
//==============================================================================
//==============================================================================
module_init(simple_init);
module_exit(simple_cleanup);
//==============================================================================
//==============================================================================
MODULE_AUTHOR("Adam Boukhssimi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realime task monitor");
