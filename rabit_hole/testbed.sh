#!/bin/sh

insmod /data/local/tmp/hello-7.ko

while true; do
    /data/local/tmp/serv_fork & servpid=$!
    /data/local/tmp/exp2
    kill -9 $servpid
    sleep 1
done
