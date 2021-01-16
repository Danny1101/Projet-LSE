#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include "log_process/log_process.h"

int main (int argc, char **argv)
{
     struct raw_log_t raw_log;
     struct log_process_t log_process;

     log_process_init();
     printf("Going into loop\n");
inf_loop:
    if(!log_process_poll(&raw_log)){
      usleep(1000);
      goto inf_loop;
    }
    log_process = log_process_compute(&raw_log);
    log_process_write(&log_process);
    log_process_cleanup_dead_tasks();
    goto inf_loop;

main_dies:
log_process_cleanup_dead_tasks();
  log_process_terminate();

  return 0;
}
