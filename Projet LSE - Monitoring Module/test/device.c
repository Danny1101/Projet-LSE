#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROCFS_MAX_SIZE 100
#define MONITORING_NODENAME "rt_monitor"

static unsigned long procfs_buffer_size = 0;
static char procfs_buffer[PROCFS_MAX_SIZE] = "Hello Adam from Kernel";
//==============================================================================
//==============================================================================
static struct proc_dir_entry *ent;
//==============================================================================
//==============================================================================
static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos)
{
	printk( KERN_DEBUG "write handler by user %s - count %d\n", ubuf, count);
	return -1;
}
//==============================================================================
//==============================================================================
static ssize_t myread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos)
{
  int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", MONITORING_NODENAME);

  procfs_buffer_size = 23;
  memcpy(ubuf, procfs_buffer, procfs_buffer_size);
  ret = procfs_buffer_size;

	return ret;
}
//==============================================================================
//==============================================================================
static struct proc_ops myops={
    .proc_read = myread,
    .proc_write = mywrite
};
//==============================================================================
//==============================================================================
static int simple_init(void)
{
	ent=proc_create(MONITORING_NODENAME,0666,NULL,&myops);
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
