#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <asm/uaccess.h>


#define JIFFIES_BUFFER_LEN 7
static char jiffies_buffer[JIFFIES_BUFFER_LEN];
static struct proc_dir_entry *proc_dir;
static int  jiffies_flag = 0;

static int 
jiffies_proc_show(struct seq_file *m, void *v)
{
    if (jiffies_flag)
    seq_printf(m, "%llu\n",
           (unsigned long long) get_jiffies_64());
    return 0;
}

static int 
jiffies_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, jiffies_proc_show, NULL);
}

static ssize_t
jiffies_proc_write(struct file *filp, const char __user *buff,
           size_t len, loff_t *data)
{
    long res;
    printk(KERN_INFO "ASE_CMD : Input from user detected\n");
    if (len > (JIFFIES_BUFFER_LEN - 1)) {
    printk(KERN_INFO "ASE_CMD: error, input too long\n");
    return -EINVAL;
    }
    else if (copy_from_user(jiffies_buffer, buff, len)) {
    return -2;
    }
    jiffies_buffer[len] = 0;

    printk(KERN_INFO "ASE_CMD: PID received : %s\n", jiffies_buffer);

    kstrtol(jiffies_buffer, 0, &res);
    jiffies_flag = res;

    return len;
}

static const struct file_operations jiffies_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = jiffies_proc_open,
    .read       = seq_read,
    .write      = jiffies_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init
jiffies_proc_init(void)
{
    proc_create("ase_cmd", 0666, NULL, &jiffies_proc_fops);
    proc_dir = proc_mkdir("ase", NULL);
    return 0;
}

static void __exit
jiffies_proc_exit(void)
{
    remove_proc_entry("ase_cmd", NULL);
}

module_init(jiffies_proc_init);
module_exit(jiffies_proc_exit);

MODULE_AUTHOR("Quentin WARNANT & Antoine Durigneux");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A ase_cmd /proc file.");
