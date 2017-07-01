TOOLCHAIN=~/toolchain
TOOLROOT=$(TOOLCHAIN)/bin/arm-linux-androideabi-
LDFLAGS += -L$(TOOLCHAIN)/sysroot/usr/lib 
CXXFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
CFLAGS += -I$(TOOLCHAIN)/sysroot/usr/include
DEST = $(HOME)/AndroidStudioProjects/Tether/app/src/main/assets/

#libmytestlib.so: mytestlib.c
#	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -shared -lselinux -o $@


%: %.c
	$(TOOLROOT)gcc -march=armv7-a -fPIE -pie $^ -lcutils -llog -lselinux -o $@

adb/%: %
	@adb push $^ /data/local/tmp/$^

all: adb/reset.sh adb/appspawn adb/tcfix appspawn tcfix 

clean:
	@rm -v tcfix appspawn 
