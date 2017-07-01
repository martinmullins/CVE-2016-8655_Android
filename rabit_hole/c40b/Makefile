TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
LDFLAGS += -L$(TOOLCHAIN)/sysroot/usr/lib 
CXXFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
CFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
libmytestlib.so: mytestlib.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -shared -lselinux -o $@


%.o: %.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie -c $^ -lcutils -llog -lselinux 

modtest5: mod.h main.o func.o pagemap.o spawn.o
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -lselinux -o $@

mod.func: ../mod_exploit/expmod.o
	sh -c ' \
	OFFSET=$$(readelf -S $^ | awk "\$$3==\".text\" {printf \"0x%s\n\", \$$6;} " ); \
	OFFSET=$$(python -c "print $$OFFSET"); \
	LEN=$$(nm -S $^ | awk "\$$4==\"lolrofl\" {printf \"0x%s\n\", \$$2;} " ); \
	LEN=$$(python -c "print $$LEN"); \
	echo $$LEN $$OFFSET; \
	dd if=$^ of=$@ bs=1 skip=$$OFFSET count=$$LEN; \
	'
	
mod.h: mod.func
	xxd --include $^ >$@

func.o: mod.h
