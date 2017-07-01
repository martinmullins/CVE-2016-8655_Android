kernel ?= $(shell uname -r)
KDIR := $(HOME)/goldfish
kdir := $(HOME)/goldfish
KVER ?= $(shell uname -r)
PWD := $(shell pwd)
TOOL_CHAIN_PATH := $(HOME)/toolchain/bin/
TOOLPREFIX := arm-linux-androideabi-
CCPATH := $(TOOL_CHAIN_PATH)$(TOOLPREFIX)
DWARFDUMP := dwarfdump

#obj-m += hello-7.o
obj-m += justexec.o

all:
	make ARCH=arm CROSS_COMPILE=$(CCPATH) EXTRA_CFLAGS=-fno-pic -C $(kdir) M=$$(pwd) modules

clean:
	make -C $(kdir) M=$$(pwd) clean

#ret2usr.ko: ret2usr.c ret2usr.h
#	$(MAKE) -C $(kdir) M=$$(pwd)
#
#ret2usr_cli: ret2usr_cli.c ret2usr.h
#	gcc -Wall $< -o $@
#
#clean:
#	rm -f ret2usr_cli *.o *.ko
