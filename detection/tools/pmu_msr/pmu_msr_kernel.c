/*
 * Record the cache miss rate of Intel Sandybridge cpu
 * To confirm the event is correctly set!
 */
#include <linux/kernel.h>
#include <linux/module.h>

#ifndef __KERNEL__
#define __KERNEL__
#endif

/*4 Performance Counters Selector for %ecx in insn wrmsr*/
#define PERFEVTSEL0    0x186
#define PERFEVTSEL1    0x187
#define PERFEVTSEL2    0x188
#define PERFEVTSEL3    0x189

/*4 MSR Performance Counter for the above selector*/
#define PMC0    0xc1
#define PMC1    0xc2
#define PMC2    0xc2
#define PMC3    0xc3

/*Intel Software Developer Manual Page 2549*/ /*L1I L1D cache events has not been confirmed!*/
/*L1 Instruction Cache Performance Tuning Events*/
#define L1I_ALLHIT_EVENT    0x80
#define L1I_ALLHIT_MASK     0x01
#define L1I_ALLMISS_EVENT   0x80    /*confirmed*/
#define L1I_ALLMISS_MASK    0x02    /*confirmed*/

#define INSTR_RETIRED_EVENT 0xc0
#define INSTR_RETIRED_MASK  0x00

/*L1 Data Cache Performance Tuning Events*/ 
/*Intel does not have the ALLREQ Miss mask; have to add LD_miss and ST_miss*/
#define L1D_ALLREQ_EVENT    0x43
#define L1D_ALLREQ_MASK     0x01
#define L1D_LDMISS_EVENT    0x40
#define L1D_LDMISS_MASK     0x01
#define L1D_STMISS_EVENT    0x28
#define L1D_STMISS_MASK     0x01

/*L2 private cache for each core*/ /*confirmed*/
#define L2_ALLREQ_EVENT     0x24
#define L2_ALLREQ_MASK      L2_ALLCODEREQ_MASK  /*0xFF*/
#define L2_ALLMISS_EVENT    0x24
#define L2_ALLMISS_MASK     L2_ALLCODEMISS_MASK /*0xAA*/

#define L2_ALLCODEREQ_MASK  0x30
#define L2_ALLCODEMISS_MASK 0x20

/*L3 shared cache*/ /*confirmed*/
/*Use the last level cache event and mask*/
#define L3_ALLREQ_EVENT     0x2E
#define L3_ALLREQ_MASK      0x4F
#define L3_ALLMISS_EVENT    0x2E
#define L3_ALLMISS_MASK     0x41 

#define USR_BIT             (0x01UL << 16)
#define OS_BIT              (0x01UL << 17)


#define SET_MSR_USR_BIT(eax)    eax |= USR_BIT
#define CLEAR_MSR_USR_BIT(exa)  eax &= (~USR_BIT)
#define SET_MSR_OS_BIT(eax)     eax |= OS_BIT
#define CLEAR_MSR_OS_BIT(eax)   eax &= (~OS_BIT)

#define SET_EVENT_MASK(eax, event, umask)    eax |= (event | (umask << 8))  

#define MSR_ENFLAG      (0x1<<22)


static inline void write_msr(uint32_t eax, uint32_t ecx)
{
   __asm__ __volatile__ (
	"movl %0, %%ecx\n\t"
        "xorl %%edx, %%edx\n\t"
        "xorl %%eax, %%eax\n\t"
        "wrmsr\n\t"
        :
        : "m" (ecx)
        : "eax", "ecx", "edx");

   eax |= MSR_ENFLAG;

   __asm__(
	"movl %0, %%ecx\n\t" 
        "xorl %%edx, %%edx\n\t"
        "movl %1, %%eax\n\t"
        "wrmsr\n\t"
        : 
        : "m" (ecx), "m" (eax)
        : "eax", "ecx", "edx");
}

static inline void read_msr(uint32_t* ecx, uint32_t *eax, uint32_t* edx)
{    __asm__ __volatile__(\
        "rdmsr"\
        :"=d" (*edx), "=a" (*eax)\
        :"c"(*ecx)
        );
}

static inline void delay(void )
{
        uint64_t tsc;
        tsc = jiffies + 10 * HZ;
        while (time_before(jiffies, tsc));
        return;
}

enum cache_level
{
    UOPS,
    L1I,
    L1D,
    L2,
    L3
};

int __init init_count_events(void) {
    enum cache_level op;
    uint32_t eax, edx, ecx;
    uint64_t l3_all;
    op = UOPS;
    switch(op)
    {
	case UOPS:
	        eax = 0x00514402;
        	eax |= MSR_ENFLAG;
	        ecx = 0x187;
	        printk(KERN_INFO "UOPS Demo: write_msr: eax=%#010x, ecx=%#010x\n", eax, ecx);
        	write_msr(eax, ecx);
        	//stop counting
		delay();
	        eax = 0x00114402;
	        write_msr(eax,ecx);
	        ecx = 0xc2;
	        eax = 1;
	        edx = 2;
	        read_msr(&ecx, &eax, &edx);
	        printk(KERN_INFO "UOPS Demo: read_msr: edx=%#010x, eax=%#010x\n", edx, eax);
        	break;
	    case L3: 
        	eax = 0;
	        SET_MSR_USR_BIT(eax);
        	SET_MSR_OS_BIT(eax);
	        SET_EVENT_MASK(eax, INSTR_RETIRED_EVENT, INSTR_RETIRED_MASK);
	        eax |= MSR_ENFLAG;
	        eax |= (1<<20); //INT bit: counter overflow
	        ecx = PERFEVTSEL2;
	        printk(KERN_INFO "before wrmsr: eax=%#010x, ecx=%#010x\n", eax, ecx);
	        write_msr(eax, ecx);
	        printk(KERN_INFO "after wrmsr: eax=%#010x, ecx=%#010x\n", eax, ecx);
	        printk(KERN_INFO "L3 all request set MSR PMC2\n");
	        printk(KERN_INFO "delay by access an array\n");
	        delay();
	        eax &= (~MSR_ENFLAG);
	        write_msr(eax, ecx);
	        printk(KERN_INFO "stop the counter, eax=%#010x\n", eax);
	        ecx = PMC2;
	        eax = 1;
	        edx = 2;
	        printk(KERN_INFO "rdmsr: ecx=%#010x\n", ecx);
	        read_msr(&ecx, &eax, &edx); /*need to pass into address!*/
	        l3_all = ( ((uint64_t) edx << 32) | eax );
	        printk(KERN_INFO "rdmsr: L3 all request is %llu (%#010lx)\n", l3_all, (unsigned long)l3_all);
	        break;
	    default:
	        printk(KERN_INFO "operation not implemented yet\n");
    }
    /* 
     * A non 0 return means init_module failed; module can't be loaded. 
     */
	return 0;
}

void __exit exit_count_events(void) {
    printk(KERN_INFO "Goodbye world 1.\n");
	return;
}

module_init(init_count_events);
module_exit(exit_count_events);

MODULE_LICENSE("GPL");
