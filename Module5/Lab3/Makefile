KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += sysfs_ioctl_blink.o
sysfs_ioctl_blink-objs := src/sysfs_ioctl_blink.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod $(PWD)/sysfs_ioctl_blink.ko

unload:
	sudo rmmod sysfs_ioctl_blink

reload: unload load
