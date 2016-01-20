#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h> 

#define FLUSH_RELOAD_CYCLE 3000
#define NR_ROUND 1000

unsigned long get_tsc_val(void)
{
	unsigned int higher32bits, lower32bits;
	__asm__ volatile("rdtsc":"=a"(lower32bits),"=d"(higher32bits));
	return lower32bits;
}

void delay(int cycles)
{
	unsigned long tsc = get_tsc_val();
	while(get_tsc_val() - tsc < cycles);
}

unsigned long probe(unsigned long addr) {
	volatile unsigned long time;
	asm __volatile__ (
	" mfence \n"
	" lfence \n"
	" rdtsc \n"
	" lfence \n"
	" movl %%eax, %%esi \n"
	" movl (%1), %%eax \n"
	" lfence \n"
	" rdtsc \n"
	" subl %%esi, %%eax \n"
	" clflush 0(%1) \n"
	: "=a" (time)
	: "c" (addr)
	: "%esi", "%edx");
	return time;
}

int main(int argc, char** argv)
{
	int fd = open("/mnt/hugepages/nebula1", O_CREAT|O_RDWR, 0755);
        unsigned long *buf = mmap(0, 1024*1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	unsigned long probe_result[NR_ROUND];
	int i;
	for (i=0; i<NR_ROUND; i++) {
		probe_result[i] = probe((unsigned long)(&buf[0]));
		delay(FLUSH_RELOAD_CYCLE);
	}

	for (i=0; i<NR_ROUND; i++)
		printf("%ld\n", probe_result[i]);

	munmap(buf, 1024*1024*1024);
	return 0;
}
