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
#define ASE_MAX_PROCT 99
#define ASE_MAX_PROCESSOR 2
static char ase_buffer[ase_BUFFER_LEN];

typedef struct proct {
	long pid;
	struct proc_dir_entry *proct_proc_file;
	struct pid *proct_pid_struct;
	struct task_struct *proct_task_struct;
	unsigned proct_start_time[ASE_MAX_PROCESSOR];
	unsigned proct_end_time[ASE_MAX_PROCESSOR];
	unsigned proct_final_time[ASE_MAX_PROCESSOR];
} proct;

static struct proc_dir_entry *proc_dir;
static int proct_count = 0;
static struct proct monitor[ASE_MAX_PROCT];

static int my_callback(struct task_struct * p) {

	int i = 0;
	for(i = 0; i < proct_count; i++){
		struct proct value = monitor[i];
		if(p->pid == value.pid){

			//delete here and move others
			int j = 0;
			for(j = i ; j < ASE_MAX_PROCT-1 ; j++){
				monitor[j] = monitor[j+1];	
			}
			proct_count--;
			break;		
		}
	}
	/* Always end with a call to jprobe_return(). */
	jprobe_return();

	return 0;
}

static struct jprobe my_jprobe = { .kp = { .symbol_name = "release_task", },
	.entry = (kprobe_opcode_t *) my_callback };

static int 
ase_proc_show(struct seq_file *m, void *v)
{
    return 0;
}

static int ase_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, ase_proc_show, NULL );
}

ssize_t ase_proc_read(struct file * file, char * buf, size_t size, loff_t * ppos) {
	struct timeval time;
	proct current_task;
	int i = 0, end_execution_time = 0, current_cpu;
	long res;

	// Get the current time
	do_gettimeofday(&time);
	end_execution_time = time.tv_sec;


	// Get the PID integer from the user input
	if(kstrtol(file->f_path.dentry->d_iname, 0, &res) != 0) {
		return 0;
	}

	// Get the current task structure
	for(i = 0; i < proct_count; i++) {
		if(res == monitor[i].pid) {
			current_task = monitor[i];
		}
	}

	current_cpu = task_cpu(current_task.proct_task_struct);

	// Compute the elapsed time
	current_task.proct_final_time[current_cpu] = end_execution_time - current_task.proct_start_time[current_cpu];
	printk(KERN_INFO "%d\n", current_task.proct_final_time[current_cpu]);

	// Write the sequence to the file
	sprintf(buf, "%d\n", current_task.proct_final_time[current_cpu]);

	return seq_read (file, buf, sizeof(int), ppos);
}

static const struct file_operations proc_current_fops = { 
	.owner = THIS_MODULE, 	
	.open = ase_proc_open, 
	.read = ase_proc_read, 
	.llseek = seq_lseek, 
	.release = single_release 
};

static ssize_t
ase_proc_write(struct file *filp, const char __user *buff,
	size_t len, loff_t *data)
{
	long res = 0;
	int start_execution_time = 0, current_cpu = 0;
	struct timeval time;
	struct proct current_track;
	struct pid *pid_struct;
	struct task_struct *current_task_struct;
	struct proc_dir_entry *proc_current_proc_file;

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
	ase_buffer[len] = '\0';

	// Get the PID integer from the user input
	if(kstrtol(ase_buffer, 0, &res) != 0) {
		return 0;
	}

	printk(KERN_INFO "ASE_CMD: PID received : %ld\n", res);

	// Get the PID structure of the current process
	pid_struct = find_get_pid(res);
	if(pid_struct == NULL) {
		printk(KERN_ERR "ASE_CMD: There is no process with %lu\n", res);
		return -3;
	}

	// Get the task structure and the name of the current process
	current_task_struct = get_pid_task(pid_struct, PIDTYPE_PID);
	current_cpu = task_cpu(current_task_struct);

	printk(KERN_INFO "ASE_CMD: Current process name %s on cpu %d: \n", current_task_struct->comm, current_cpu);

	if(proc_dir == NULL) {
		printk(KERN_ERR "ASE_CMD: Unable to create current proc file in ase directory\n");
		return -4;
	}

	printk(KERN_INFO "ASE_CMD: Create current proc file in ase directory\n");
	proc_current_proc_file = proc_create(ase_buffer,0666,proc_dir, &proc_current_fops);

	printk(KERN_INFO "ASE_CMD: plant jprobe at %p\n", my_jprobe.entry);

	current_track.proct_proc_file = proc_current_proc_file;
	current_track.proct_pid_struct = pid_struct;
	current_track.proct_task_struct = current_task_struct;
	current_track.proct_start_time[current_cpu] = start_execution_time;
	current_track.pid = res;

	monitor[proct_count++] = current_track;

	return len;
}

static const struct file_operations ase_proc_fops = { 
	.owner = THIS_MODULE,
	.open = ase_proc_open,
	.read = seq_read, 
	.write = ase_proc_write, 
	.llseek = seq_lseek, 
	.release = single_release
};

static int __init
ase_proc_init(void) {

	int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "ASE_CMD: register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "ASE_CMD: Planted jprobe at %p, handler addr %p\n",my_jprobe.kp.addr, my_jprobe.entry);

	proc_create("ase_cmd", 0666, NULL, &ase_proc_fops);
	proc_dir = proc_mkdir_data("ase", 0777, NULL, NULL );

	if(proc_dir == NULL)
	printk(KERN_ERR "ASE_CMD: Failed to create the ase proc directory\n");

	return 0;
}

static void __exit
ase_proc_exit(void) {
	unregister_jprobe(&my_jprobe);

	printk(KERN_INFO "ASE_CMD: Cleanup ase_cmd module\n");

	remove_proc_entry("ase_cmd", NULL );
	printk(KERN_INFO "ASE_CMD: ase_cmd file removed\n");

	remove_proc_subtree("ase", NULL );
	printk(KERN_INFO "ASE_CMD: ase folder removed\n");
}

module_init( ase_proc_init);
module_exit( ase_proc_exit);

MODULE_AUTHOR("Quentin WARNANT & Antoine Durigneux");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A ase_cmd /proc file.");
