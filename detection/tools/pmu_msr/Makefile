KDIR := /lib/modules/$(shell uname -r)/build
CC = gcc

ERROR_FLAGS = -std=gnu99 -Wall -Werror -O0 -g
CFLAGS1 = $(ERROR_FLAGS) -D_GNU_SOURCE
LDFLAGS1 = $(ERROR_FLAGS)

obj-m += pmu_msr_kernel.o

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

user: pmu_msr_user.c
	$(CC) $(CFLAGS1) -o pmu_msr_user pmu_msr_user.c $(LDFLAGS1)

clean:
	rm -rf *.o pmu_msr_user *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
