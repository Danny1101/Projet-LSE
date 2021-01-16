#ifndef __SCHED_DEADLINE_H
#define __SCHED_DEADLINE_H
#include <stdint.h>

#define TASK_MONITOR_DIRECTORY  "/task_monitor"
#define TASK_MONITOR_EXPORT_FIFO  "export"

struct sched_deadline_params_t{
  pid_t pid;

  uint64_t period_us;
  uint64_t deadline_us;
  uint64_t normal_wcet_us;
  uint64_t abnormal_wcet_us;
};

struct custom_task_struct_t{
  // Scheduling infos
  /* # of times we have run on this CPU: */
	unsigned long			pcount;

	/* Time spent waiting on a runqueue: */
	unsigned long long		run_delay;

	/* Timestamps: */

	/* When did we last run on a CPU? */
	unsigned long long		last_arrival;

	/* When were we last queued to run? */
	unsigned long long		last_queued;

  uint64_t utime;
  uint64_t stime;
  /* Context switch counts: */
  unsigned long			nvcsw;
	unsigned long			nivcsw;

};

struct rt_monitor_sig_data_t{
  int pid;
  struct custom_task_struct_t* custom_task_struct;
};

#endif
