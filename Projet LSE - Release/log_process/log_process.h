#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <sys/times.h>
#include "../log/log.h"

#define TASK_MONITOR_DIR "/task_monitor"
#define MONITORED_PROCESSES_MAX 128
struct log_process_t{
  pid_t pid;
  char* logpath;
  long syscall;

  int iteration;

  struct custom_time_t offset;
  struct custom_time_t preloop;
  struct custom_time_t postloop;
  struct custom_time_t usage;

  struct custom_time_t user_mode_time;
  struct custom_time_t kernel_mode_time;

  unsigned long long run_delay;
  unsigned long pcount;
  struct custom_time_t last_arrival;
  unsigned long long last_queued;

  uint64_t vcsw;
  uint64_t ivcsw;

  struct custom_time_t execution_time;
};
//==============================================================================
//============== Functions to be called in the monitoring process ==============
void log_process_init();

int log_process_poll(struct raw_log_t* storein);
struct log_process_t log_process_compute(struct raw_log_t*);
void log_process_write(struct log_process_t*);

void log_process_cleanup_dead_tasks();
void log_process_terminate();
