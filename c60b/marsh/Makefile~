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

$(DEST)%: %
	@cp -pv $^ $@

all: $(DEST)reseta.sh $(DEST)reset.sh $(DEST)appspawn $(DEST)setprop $(DEST)reset reset appspawn setprop
