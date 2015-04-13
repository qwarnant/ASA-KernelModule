#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/kprobes.h>
#include <linux/time.h>


#define ase_BUFFER_LEN 7
static char ase_buffer[ase_BUFFER_LEN];

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_current_proc_file;

static int proc_current_pid = 0;
static struct pid *pid_struct;
static struct task_struct *current_task_struct;

// In seconds
static unsigned start_execution_time = 0;  
static unsigned end_execution_time = 0; 
static int final_execution_time = 0;

static int my_callback(struct task_struct * p) {
       

	struct timeval time;
	
	if(p->pid == proc_current_pid) {

	   do_gettimeofday(&time);
	   end_execution_time = time.tv_sec;

	   final_execution_time = end_execution_time - start_execution_time;
	   printk(KERN_INFO "ASE_CMD: Execution time of process %s : %d\n", current_task_struct->comm, final_execution_time);
	}
  
/* Always end with a call to jprobe_return(). */
        jprobe_return();
      
        return 0;
}

static struct jprobe my_jprobe = {
    .kp = {
    .symbol_name = "release_task",
    },
    .entry = (kprobe_opcode_t *) my_callback
};


static int 
ase_proc_show(struct seq_file *m, void *v)
{
	/*struct timeval time;
	
	if(p->pid == proc_current_pid) {

	   do_gettimeofday(&time);
	   end_execution_time = time.tv_sec;

	   final_execution_time = end_execution_time - start_execution_time;
	   printk(KERN_INFO "ASE_CMD: Execution time of process %s : %d\n", current_task_struct->comm, final_execution_time);
	}
*/
    return 0;
}

static int 
ase_proc_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "ASE_CMD: ase_proc_show %s\n", (char*) file->f_path.dentry->d_iname);
    return single_open(file, ase_proc_show, NULL);
}

static const struct file_operations proc_current_fops = {
    .owner      = THIS_MODULE,
    .open       = ase_proc_open
};

static ssize_t
ase_proc_write(struct file *filp, const char __user *buff,
           size_t len, loff_t *data)
{
    long res;
    struct timeval time;
    do_gettimeofday(&time);
    start_execution_time = time.tv_sec;

    // Get the process ID from the user
    printk(KERN_INFO "ASE_CMD : Input from user detected\n");
    if (len > (ase_BUFFER_LEN - 1)) {
    printk(KERN_INFO "ASE_CMD: error, input too long\n");
    return -EINVAL;
    }
    else if (copy_from_user(ase_buffer, buff, len-1)) {
    return -2;
    }
    ase_buffer[len] = 0;

    kstrtol(ase_buffer, 0, &res);

    proc_current_pid = res;
    printk(KERN_INFO "ASE_CMD: PID received : %ld\n", proc_current_pid);

    // Get the PID structure of the current process
    pid_struct = find_get_pid(proc_current_pid);
    if(pid_struct == NULL) {
    	printk(KERN_ERR "ASE_CMD: There is no process with %d\n", proc_current_pid);
	return -3;
    }

    // Get the task structure and the name of the current process
    current_task_struct = get_pid_task(pid_struct, PIDTYPE_PID);
    printk(KERN_INFO "ASE_CMD: Current process name : %s\n", current_task_struct->comm);

    if(proc_dir != NULL) {
    	printk(KERN_INFO "ASE_CMD: Create current proc file in ase directory\n");
    	proc_current_proc_file = proc_create(ase_buffer,0666,proc_dir, &proc_current_fops);	

        printk(KERN_INFO "ASE_CMD: plant jprobe at %p\n", my_jprobe.entry);

    }
    return len;
}

static const struct file_operations ase_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = ase_proc_open,
    .read       = seq_read,
    .write      = ase_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init
ase_proc_init(void)
{
    
int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
	  printk(KERN_INFO "ASE_CMD: register_jprobe failed, returned %d\n", ret);
	  return -1;
	}
	printk(KERN_INFO "ASE_CMD: Planted jprobe at %p, handler addr %p\n",my_jprobe.kp.addr, my_jprobe.entry);

    proc_create("ase_cmd", 0666, NULL, &ase_proc_fops);
    proc_dir = proc_mkdir_data("ase", 0777, NULL, NULL);

    if(proc_dir == NULL)
	printk(KERN_ERR "ASE_CMD: Failed to create the ase proc directory\n");

    return 0;
}

static void __exit
ase_proc_exit(void)
{
    unregister_jprobe(&my_jprobe);

    printk(KERN_INFO "ASE_CMD: Cleanup ase_cmd module\n");

    remove_proc_entry("ase_cmd", NULL);
    printk(KERN_INFO "ASE_CMD: ase_cmd file removed\n");

    remove_proc_subtree("ase", NULL);
    printk(KERN_INFO "ASE_CMD: ase folder removed\n");
}

module_init(ase_proc_init);
module_exit(ase_proc_exit);

MODULE_AUTHOR("Quentin WARNANT & Antoine Durigneux");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A ase_cmd /proc file.");
