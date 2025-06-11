KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += chardev_rw.o
chardev_rw-objs := src/chardev_rw.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod $(PWD)/chardev_rw.ko

unload:
	sudo rmmod chardev_rw

reload: unload load
