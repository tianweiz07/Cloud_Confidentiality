KDIR := /lib/modules/$(shell uname -r)/build
CC = gcc

obj-m += add_syscall.o

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

user:
	$(CC) -o user user.c

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp* user
