#ifndef __LOG_H
#define __LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../monitor_headers/sched_deadline.h"

#define LOG_FIFO_PATH TASK_MONITOR_DIRECTORY "/log_fifo"

//=================================================
struct custom_time_t{
  int s;
  int ms;
  int us;
  int ns;
};
//=================================================
// Only one will be sent each iteration
struct raw_log_t{

  // Process identifier
  pid_t pid;

  // timings
  struct timespec offset;
  struct timespec preloop;
  struct timespec postloop;

  // Process raw execution variables
  long syscall;
  uint64_t utime;
  uint64_t stime;

  unsigned long long		run_delay;
  unsigned long			pcount;
  unsigned long long		last_arrival;
  unsigned long long		last_queued;

  uint64_t vcsw;
  uint64_t ivcsw;
};
//==============================================================================
//============== Functions to be called in a time critical context =============
void log_init(struct raw_log_t*);

void log_offset_mark(struct raw_log_t*);
void log_preloop_mark(struct raw_log_t*);
void log_postloop_mark(struct raw_log_t*);

void log_free(struct raw_log_t*);
#endif
