CC=gcc
CFLAGS  = -g -Wall -lrt -pthread


mtask_launch: log/log.o syscalls/syscalls.o trace/trace.o
	$(CC) $(CFLAGS) mtask_launch.c -o mtask_launch log/log.o syscalls/syscalls.o trace/trace.o

test: syscalls/syscalls.o
	$(CC) $(CFLAGS) test.c -o _test syscalls/syscalls.o

monitor_log: log_process/log_process.o
		$(CC) $(CFLAGS) monitor_log.c -o monitor_log log_process/log_process.o

#================================================================================================================
#================================================================================================================
#================================================================================================================

log.o: log/log.h log/log.c
	$(CC) $(CFLAGS) log/log.c

syscalls.o: syscalls/syscalls.h syscalls/syscalls.c
	$(CC) $(CFLAGS) syscalls/syscalls.c

trace.o: trace/trace.h trace/trace.c
	$(CC) $(CFLAGS) trace/trace.c

clean:
	@echo "Cleaning up"
	rm -v fifo_in/fifo_in.o syscalls/syscalls.o trace/trace.o log/log.o log_process/log_process.o
