KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += proc_msg.o
proc_msg-objs := src/proc_msg.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	