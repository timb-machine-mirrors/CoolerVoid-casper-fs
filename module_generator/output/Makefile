CONFIG_MODULE_SIG=n
PWD := $(shell pwd)
CC := gcc
KERNEL_PATH ?= /lib/modules/$(shell uname -r)/build
ccflags-y	+= -Wall -Wdeclaration-after-statement

obj-m += casperfs.o
casperfs-objs := main.o hooked.o 

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
