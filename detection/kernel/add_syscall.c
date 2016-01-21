#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <asm/io.h>

#ifndef __KERNEL__
#define __KERNEL__
#endif

static uint64_t SYS_CALL_TABLE_ADDR = 0xffffffff81801400;
#define __NR_Check 189

#define CPU_LOWER_BOUND 0
#define CPU_UPPER_BOUND 15
#define NUM_THREAD 1

void **my_sys_call_table;
static uint64_t (*position_collect)(void);

static void disable_page_protection(void) {
    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (value & 0x00010000) {
            value &= ~0x00010000;
            asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

static void enable_page_protection(void) {
    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (!(value & 0x00010000)) {
            value |= 0x00010000;
            asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

asmlinkage void sys_Check(int *x) {
	int i;
	struct task_struct *task;
	for (i=0; i<NUM_THREAD; i++) {
		task = pid_task(find_vpid(x[i]), PIDTYPE_PID);
		if ((task->state == 0) && (task->wake_cpu <= CPU_UPPER_BOUND) && (task->wake_cpu >= CPU_LOWER_BOUND))
			continue;
		else
			x[i] = -1;
	}
}

int __init init_addsyscall(void) {
        my_sys_call_table = (void**) SYS_CALL_TABLE_ADDR;
	disable_page_protection();
        position_collect = (uint64_t(*)(void))(my_sys_call_table[__NR_Check]);
        my_sys_call_table[__NR_Check] = sys_Check;
	enable_page_protection();
	return 0;
}

void __exit exit_addsyscall(void) {
	printk("Module Exit\n");
	disable_page_protection();
        my_sys_call_table[__NR_Check] = position_collect;
	enable_page_protection();
	return;
}

module_init(init_addsyscall);
module_exit(exit_addsyscall);

MODULE_LICENSE("GPL");
