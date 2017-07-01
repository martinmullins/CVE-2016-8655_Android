#!/bin/bash
# don't need dirtycow on goldfish

echo "Running on port 5000"
adb shell /data/local/tmp/serv_fork 5000 &
adb shell /data/local/tmp/c50b $port

echo "cat /sys/kernel/debug/tracing/trace"
echo "Done"
