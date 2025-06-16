KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += netlink_kernel.o
netlink_kernel-objs := src/netlink_kernel.o

all: modules client

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

client:
	$(CC) -Wall -o client src/netlink_user.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) client

load:
	sudo insmod $(PWD)/netlink_kernel.ko

unload:
	sudo rmmod netlink_kernel

reload: unload load
