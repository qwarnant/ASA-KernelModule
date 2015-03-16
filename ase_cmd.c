#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>


static int my_callback(pid_t pid, int policy,
                              const struct sched_param *param) {
	printk(KERN_ALERT "My callback is called\n");
	return 0;
}

static struct jprobe my_jprobe = {
    .kp = {
    .symbol_name = "sched_setscheduler",
    },
    .entry = (kprobe_opcode_t *) my_callback
};


static int __init kprobes_proc_init(void) {
	register_jprobe(&my_jprobe);
	printk(KERN_ALERT "plant jprobe at %p, handler addr %p\n",
       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

static int __exit kprobes_proc_exit(void) {
	unregister_jprobe(&my_jprobe);
	printk(KERN_ALERT "My callback is exit\n");
	return 0;
}

module_init(kprobes_proc_init);
module_exit(kprobes_proc_exit);

MODULE_AUTHOR("Quentin WARNANT");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A ase_cmd /proc file.");
