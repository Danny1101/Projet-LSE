#include "trace.h"
//==============================================================================
//==============================================================================
static void __handle_syscall_entry(struct pt_regs , struct raw_log_t* log, int dev, pid_t cpid);
static void __handle_sched_yield(long is_entry, struct raw_log_t* log, int dev);
static void __handle_sched_sleep(struct pt_regs , struct raw_log_t* log, int dev, pid_t cpid);
static void __read_vitals(int dev, struct custom_task_struct_t* vitals);
//==============================================================================
//==============================================================================
void trace_child(pid_t child_pid, struct raw_log_t* log, struct sched_deadline_params_t* params){
  int status;
  struct pt_regs regs;
  int dev;

  wait(&status);
  ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_EXITKILL);
  if(!(dev = open("/proc/rt_monitor", O_RDONLY))){
      perror("open()");
      return;
  }

  log_offset_mark(log);

  while(status == 1407){
    memset(&regs, 0, sizeof(struct pt_regs));
    ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
    __handle_syscall_entry(regs, log, dev, child_pid);
    ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
    wait(&status);
  }

  printf("======> Dead process\n");
  close(dev);
}
//==============================================================================
//==============================================================================
static void __handle_syscall_entry(struct pt_regs regs, struct raw_log_t* log, int dev, pid_t cpid){
  struct custom_task_struct_t vitals;
  static int iteration = 0;

  if(SYSCALL == __NR_sched_yield){
    __handle_sched_yield(SYSCALL_ENTRY, log, dev);
  }else if((SYSCALL == __NR_clock_nanosleep_time64) ||
            (SYSCALL == SYS__newselect)){
    __handle_sched_sleep(regs, log, dev, cpid);
  }
}
//==============================================================================
//==============================================================================
static void __handle_sched_sleep(struct pt_regs regs, struct raw_log_t* log, int dev, pid_t cpid){
  static int iteration = 0;
  struct custom_task_struct_t vitals;

  if(SYSCALL_ENTRY){
    // Going into sleep ...
    // Must evaluate usage for this iteration
    log_preloop_mark(log);

    __read_vitals(dev, &vitals);
    log->syscall = SYSCALL;
    log->utime = vitals.utime;
    log->stime = vitals.stime;
    log->run_delay = vitals.run_delay;
    log->pcount = vitals.pcount;
    log->last_arrival = vitals.last_arrival;
    log->last_queued = vitals.last_queued;
    log->vcsw = vitals.nvcsw;
    log->ivcsw = vitals.nivcsw;
    iteration++;
    log_postloop_mark(log);
  }else{

  }
}
//==============================================================================
//==============================================================================
static void __handle_sched_yield(long is_entry, struct raw_log_t* log, int dev){
  static int iteration = 0;
  struct custom_task_struct_t vitals;

  if (is_entry) {
    // sched_yield() called, time to sample the statistics of the children task
    // The child is going into sleep until next period
    log_preloop_mark(log);
    __read_vitals(dev, &vitals);
    log->utime = vitals.utime;
    log->stime = vitals.stime;
    log->run_delay = vitals.run_delay;
    log->pcount = vitals.pcount;
    log->last_arrival = vitals.last_arrival;
    log->last_queued = vitals.last_queued;
    log->vcsw = vitals.nvcsw;
    log->ivcsw = vitals.nivcsw;
    iteration++;
    log_postloop_mark(log);
    /*
    Now the monitor must kill the child process {log->pid} if he exceeds its deadline OR its abnormal WCET
    */
  }else{
    // The task is going to wakeup

  }
}
//==============================================================================
//==============================================================================
static void __read_vitals(int dev, struct custom_task_struct_t* vitals){
  memset(vitals, 0, sizeof(struct custom_task_struct_t));
  if(read(dev, vitals, sizeof(struct custom_task_struct_t)) == -1){
    perror("read()");
    exit(EXIT_FAILURE);
  }
}
//==============================================================================
//==============================================================================
