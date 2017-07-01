TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
LDFLAGS += -L$(TOOLCHAIN)/sysroot/usr/lib 
CXXFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
CFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
libmytestlib.so: mytestlib.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -shared -lselinux -o $@


%: %.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -lselinux -o $@
