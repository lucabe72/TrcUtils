get_debugfs() {
  RES=$(mount | grep debugfs | cut -d ' ' -f 3)
  if [ x$RES = x ]
   then
    echo >&2 DebugFS not mounted!!!
    sudo mkdir -p /sys/kernel/debug
    sudo mount -t debugfs debugfd /sys/kernel/debug
    if [ $? = 0 ]
     then
      echo /sys/kernel/debug
      exit 0
     else
      echo >&2 Cannot mount DebugFS!!! Please enable debugfs and ftrace in the kernel config!
      exit 1
     fi
   fi
  echo $RES
}


DEBUGFS=$(get_debugfs)
if [ $? -eq 1 ]
 then
  exit 1
 fi
TRACINGFS=$DEBUGFS/tracing

echo 0 > $TRACINGFS/tracing_enabled
#cat $TRACINGFS/trace_pipe > /dev/null

echo "sched_switch sched_wakeup sched_migrate_task" > $TRACINGFS/set_event

echo 1 > $TRACINGFS/tracing_enabled
echo sleeping
sleep 30
echo slept
echo 0 > $TRACINGFS/tracing_enabled

cat $TRACINGFS/trace_pipe > $1
