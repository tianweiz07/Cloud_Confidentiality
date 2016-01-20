#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define SIZE 20971520
#define LINE 64
int *source_array;
int *dest_array;

uint64_t rdtsc(void) {
	uint64_t a, d;
	__asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
	return (d<<32) | a;
}

void init_stream() {
	source_array = mmap(0, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	dest_array = mmap(0, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	int i;
	for (i=0; i <SIZE/sizeof(int); i++) {
		source_array[i] = rand();
		dest_array[i] = rand();
	}
}

void run_stream() {
	int j;
//	uint64_t interval = 300000000*(1+rand()%10);
	uint64_t interval = 3000000000;
	uint64_t end = rdtsc() + interval;
	while (rdtsc() < end) {
		for (j=0; j < SIZE/LINE; j++) {
	                dest_array[j*LINE/sizeof(int)] = source_array[j*LINE/sizeof(int)];
		}
	}
}


int main() {
	pid_t pid = syscall(__NR_getpid);
	printf("pid: %d\n", pid);
	init_stream();
	int status;
	while (1) {
		status = system("gpg --batch --passphrase adminj310a -r FD2910E9 -d \"plaintext.gpg\"");
		run_stream();
	}
	return 0;
}
