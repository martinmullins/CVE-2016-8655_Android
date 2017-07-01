#!/bin/bash

emupid=0

startemuOK() {
    ~/android-sdk-linux/tools/emulator -avd asdf -no-window -no-boot-anim -kernel /home/marto/zImage -show-kernel -debug init -sdcard /home/marto/sdcard2 &>$1 & emupid=$!
}

for i in {1..2}; do
    startemuOK "/tmp/kern_${i}.log"
    if [ $emupid -eq 0 ]; then
        echo "failed to start emu"
        exit 1
    fi

    adb wait-for-device

    adb shell sh /data/local/tmp/testbed.sh >/tmp/testbed_${i}.log & shellpid=$!

    while true; do
        if ps --pid=$emupid; then 
            sleep 10
            continue
            #echo "killing the emulator..."
            #sleep 10
            #killemu
            #kill -9 $shellpid
            #kill -9 $emupid
            #pkill -9 adb 
            #break
        fi
        break
    done
done
