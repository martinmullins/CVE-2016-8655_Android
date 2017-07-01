TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
LDFLAGS += -L$(TOOLCHAIN)/sysroot/usr/lib 
CXXFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
CFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
libmytestlib.so: mytestlib.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -shared -lselinux -o $@


%.o: %.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie -c $^ -lcutils -llog -lselinux 

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

exp2: mod.h exp.o pagemap.o func.o race.o checkexe.o
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -lselinux -o $@

lol.func: func.o
	sh -c ' \
	OFFSET=$$(readelf -S func.o | awk "\$$3==\".text\" {printf \"0x%s\n\", \$$6;} " ); \
	OFFSET=$$(python -c "print $$OFFSET"); \
	LEN=$$(nm -S func.o | awk "\$$4==\"lolrofl\" {printf \"0x%s\n\", \$$2;} " ); \
	LEN=$$(python -c "print $$LEN"); \
	echo $$LEN $$OFFSET; \
	dd if=func.o of=lol.func bs=1 skip=$$OFFSET count=$$LEN; \
	'

lol.h: lol.func
	xxd --include $^ >$@

functest: functest.c lol.h
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $@.c -lcutils -llog -lselinux  -o $@
