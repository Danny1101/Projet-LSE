#include "log_process.h"
static struct raw_log_t last_values[MONITORED_PROCESSES_MAX];
static int iterations[MONITORED_PROCESSES_MAX];
static char filepaths[MONITORED_PROCESSES_MAX][128];
//==============================================================================
int fifo_log;
static int __get_proc_name(pid_t pid, char* buffer);
//==============================================================================
//==============================================================================
static int __get_last_value_index(pid_t of){
  for (int i = 0; i < MONITORED_PROCESSES_MAX; i++) {
    if(last_values[i].pid == of)
      return i;
  }

  return -1;
}
//==============================================================================
static struct raw_log_t __update_last_value(struct raw_log_t* elem){
  struct raw_log_t previous;
  previous.pid = elem->pid;
  previous.utime = 0;
  previous.stime = 0;
  previous.vcsw = 0;
  previous.ivcsw = 0;
  previous.run_delay = 0;
  previous.pcount = 0;
  previous.last_arrival = 0;
  previous.last_queued = 0;
  int index = __get_last_value_index(elem->pid);
  if(index == -1){
    // Create new one
    if((index = __get_last_value_index(-1)) == -1)
      return previous;
    iterations[index] = 0;
    __get_proc_name(previous.pid, filepaths[index]);
  }else{
    // Save the previous value
    previous = last_values[index];
    iterations[index]++;
  }
  last_values[index] = *elem;
  return previous;
}
//==============================================================================
void log_process_cleanup_dead_tasks(){
  struct stat sts;
  char proc_path[128] = "/proc/";
  char task_path_old[128],task_path_new[128];

  for (int i = 0; i < MONITORED_PROCESSES_MAX; i++) {
    sprintf(proc_path, "/proc/%d/stat", last_values[i].pid);
    if(last_values[i].pid == -1)continue;
    if(stat(proc_path, &sts) == -1 && errno == ENOENT){
      // Dead process
      last_values[i].pid = -1;
      last_values[i].utime = 0;
      last_values[i].stime = 0;
      last_values[i].vcsw = 0;
      last_values[i].ivcsw = 0;

      last_values[i].run_delay = 0;
      last_values[i].pcount = 0;
      last_values[i].last_arrival = 0;
      last_values[i].last_queued = 0;

      iterations[i] = 0;
      sprintf(task_path_old, TASK_MONITOR_DIRECTORY "/%d", last_values[i].pid);
      sprintf(task_path_new, TASK_MONITOR_DIRECTORY "/%d [DEAD]", last_values[i].pid);
      printf("rename {%s} to {%s}\n", task_path_old, task_path_new);
      rename(task_path_old, task_path_new);
    }
  }
}
//==============================================================================
static struct custom_time_t __conv_timespec_to_custom(struct timespec* elem, struct timespec* offset){
  struct custom_time_t out;
  if(offset){
    uint64_t diff_nsec = 0;
    uint64_t diff_sec = elem->tv_sec - offset->tv_sec;
    if(elem->tv_nsec < offset->tv_nsec){
      diff_nsec = 1000000000;
      diff_sec--;
    }
    diff_nsec += elem->tv_nsec - offset->tv_nsec;

    out.s = diff_sec;
    out.ms = diff_nsec/1000000;
    out.us = (diff_nsec/1000)%1000;
    out.ns = diff_nsec%1000;
  }else{
    out.s = elem->tv_sec;
    out.ms = elem->tv_nsec/1000000;
    out.us = (elem->tv_nsec/1000)%1000;
    out.ns = elem->tv_nsec%1000;
  }
  return out;
}
//==============================================================================
void log_process_init(){
  fifo_log = open(LOG_FIFO_PATH, O_RDONLY);
  for(int i=0; i < MONITORED_PROCESSES_MAX ; i++){
    last_values[i].pid = -1;
    iterations[i] = 0;
    last_values[i].utime = 0;
    last_values[i].stime = 0;
    last_values[i].vcsw = 0;
    last_values[i].ivcsw = 0;
    last_values[i].run_delay = 0;
    last_values[i].pcount = 0;
    last_values[i].last_arrival = 0;
    last_values[i].last_queued = 0;
  }
}
//==============================================================================
int log_process_poll(struct raw_log_t* storein){
  return read(fifo_log, storein, sizeof(struct raw_log_t));
}
//==============================================================================
struct log_process_t log_process_compute(struct raw_log_t* raw_log){
  uint64_t time_operations, sum;
  struct log_process_t out;
  struct raw_log_t previous = __update_last_value(raw_log);
  struct timespec monitoring_usage;

  out.iteration = __get_last_value_index(out.pid = raw_log->pid);
  out.logpath = filepaths[out.iteration];
  out.iteration = iterations[out.iteration];

  sum = time_operations = (raw_log->utime - previous.utime);
  out.user_mode_time.s = time_operations/1000000000;
  out.user_mode_time.ms = (time_operations/1000000)%1000;
  out.user_mode_time.us = (time_operations/1000)%1000;
  out.user_mode_time.ns = time_operations%1000;

  time_operations = (raw_log->stime - previous.stime);
  sum += time_operations;
  out.kernel_mode_time.s = time_operations/1000000000;
  out.kernel_mode_time.ms = (time_operations/1000000)%1000;
  out.kernel_mode_time.us = (time_operations/1000)%1000;
  out.kernel_mode_time.ns = time_operations%1000;

  out.vcsw = raw_log->vcsw - previous.vcsw;
  out.ivcsw = raw_log->ivcsw - previous.ivcsw;

  out.offset = __conv_timespec_to_custom(&raw_log->offset, NULL);
  out.preloop = __conv_timespec_to_custom(&raw_log->preloop, &raw_log->offset);
  out.postloop = __conv_timespec_to_custom(&raw_log->postloop, &raw_log->offset);

  out.execution_time.s  = sum/1000000000;
  out.execution_time.ms = (sum/1000000)%1000;
  out.execution_time.us = (sum/1000)%1000;
  out.execution_time.ns = sum%1000;

  out.run_delay = raw_log->run_delay - previous.run_delay;
  out.pcount = raw_log->pcount - previous.pcount;
  out.last_queued = raw_log->last_queued;

  out.last_arrival.s = raw_log->last_arrival/1000000000;
  out.last_arrival.ms = (raw_log->last_arrival/1000000)%1000;
  out.last_arrival.us = (raw_log->last_arrival/1000)%1000;
  out.last_arrival.ns = (raw_log->last_arrival%1000);

  monitoring_usage.tv_sec = raw_log->postloop.tv_sec - raw_log->preloop.tv_sec;
  if(raw_log->postloop.tv_nsec < raw_log->preloop.tv_nsec){
    monitoring_usage.tv_sec--;
    monitoring_usage.tv_nsec = 1000000000 - raw_log->preloop.tv_nsec;
    monitoring_usage.tv_nsec += raw_log->postloop.tv_nsec;
  }else
    monitoring_usage.tv_nsec = raw_log->postloop.tv_nsec - raw_log->preloop.tv_nsec;

  out.usage = __conv_timespec_to_custom(&monitoring_usage, NULL);

  out.syscall = raw_log->syscall;

  return out;
}
//==============================================================================
//==============================================================================
static int __get_proc_name(pid_t pid, char* buffer){
  FILE* s_stream;
  char _path[128];
  int iter=0;
  sprintf(_path, "/proc/%d/stat", pid);

  if(!(s_stream=fopen(_path, "r")))
    return 1;

  while(getc(s_stream) != ' ');
  getc(s_stream); // ignore the first (
  while( (_path[iter++] = getc(s_stream)) != ')');
  _path[iter-1] = '\0';
  sprintf(buffer, TASK_MONITOR_DIR "/[%d] %s.log", pid, _path);
  fclose(s_stream);
  return 0;
}
//==============================================================================
void log_process_write(struct log_process_t* log){
  char path[128];
  FILE* flog = stdout;

  if( !(flog=fopen(log->logpath, "a")) ){
    perror("flog=fopen()");
    return;
  }

  if(log->iteration){
    fprintf(flog, "===================== Iteration %d - Syscall = 0x%lx =====================\n", log->iteration, log->syscall);
    fprintf(flog, "[%.3d %.3d %.3d %.3d] : Monitoring loop started !\n", log->preloop.s, log->preloop.ms, log->preloop.us, log->preloop.ns);
  }else{
    fprintf(flog, "[%.3d %.3d %.3d %.3d] : Monitoring started, fixing this time as offset !\n", log->offset.s, log->offset.ms, log->offset.us, log->offset.ns);
  }
  fprintf(flog, "Execution times :\n");
  fprintf(flog, "\tUser mode execution time %.3ds %.3dms %.3dus %.3dns\n", log->user_mode_time.s, log->user_mode_time.ms, log->user_mode_time.us, log->user_mode_time.ns);
  fprintf(flog, "\tKernel mode execution time %.3ds %.3dms %.3dus %.3dns\n", log->kernel_mode_time.s, log->kernel_mode_time.ms, log->kernel_mode_time.us, log->kernel_mode_time.ns);
  fprintf(flog, "\tTotal execution time %.3ds %.3dms %.3dus %.3dns\n", log->execution_time.s, log->execution_time.ms, log->execution_time.us, log->execution_time.ns);

  fprintf(flog, "Context switch :\n");
  fprintf(flog, "\tVoluntary context switches : %lld\n", log->vcsw);
  fprintf(flog, "\tInvoluntary context switches : %lld\n", log->ivcsw);

  fprintf(flog, "Scheduling informations :\n");
  fprintf(flog, "\tNumber of times we have run on this CPU : %lld\n", log->run_delay);
  fprintf(flog, "\tTime spent waiting on runqueue : %ld\n", log->pcount);
  fprintf(flog, "\tLast run on a CPU : %ds %.3dms %.3dus %.3dns\n", log->last_arrival.s, log->last_arrival.ms, log->last_arrival.us, log->last_arrival.ns);
  fprintf(flog, "\tLast queued to run : %lld\n", log->last_queued);


  fprintf(flog, "Monitoring duration : %ds %.3dms %.3dus %.3dns\n", log->usage.s, log->usage.ms, log->usage.us, log->usage.ns);
  fprintf(flog, "[%.3d %.3d %.3d %.3d] : Monitoring loop ended !\n\n", log->postloop.s, log->postloop.ms, log->postloop.us, log->postloop.ns);

  fclose(flog);
}
//==============================================================================
void log_process_terminate(){
  close(fifo_log);
}
