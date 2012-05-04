DEBUGFS=/mnt
TRACINGFS=$DEBUGFS/tracing

echo 0 > $TRACINGFS/tracing_enabled
#cat $TRACINGFS/trace_pipe > /dev/null

echo "sched_switch sched_wakeup sched_migrate_task" > $TRACINGFS/set_event

echo 1 > $TRACINGFS/tracing_enabled

mkfifo /tmp/fifo
cat $TRACINGFS/trace_pipe | ./import - 2> log > /tmp/fifo &
./visual /tmp/fifo 
