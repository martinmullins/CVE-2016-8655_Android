obj-m += module.o
KDIR := $(HOME)/goldfish
KVER ?= $(shell uname -r)
PWD := $(shell pwd)
TOOL_CHAIN_PATH := $(HOME)/toolchain/bin/
TOOLPREFIX := arm-linux-androideabi-
CCPATH := $(TOOL_CHAIN_PATH)$(TOOLPREFIX)
DWARFDUMP := dwarfdump

#EXTRA_CFLAGS := -I/home/marto/goldfish/net/packet/

-include version.mk

all: dwarf 

dwarf: module.c
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CCPATH) -C $(KDIR) CONFIG_DEBUG_INFO=y M="$(PWD)" modules
	$(DWARFDUMP) -di module.ko > module.dwarf
	#$(MAKE) -C $(KDIR)/lib/modules/$(KVER)/build M="$(PWD)" clean

clean:
	$(MAKE) -C $(KDIR)/lib/modules/$(KVER)/build M="$(PWD)" clean
	rm -f module.dwarf
