#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/syscall.h>

#define __NR_Check 189

int main(int argc, char* argv[]) {
	int x[4];
	x[0] = 2720;
	x[1] = 2722;
	x[2] = 2723;
	x[3] = 2726;

	syscall(__NR_Check, x);

	int i;
	for (i=0; i<4; i++)
		printf("%d\n", x[i]);
	return 0;
}
