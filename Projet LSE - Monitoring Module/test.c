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
#include "syscalls/syscalls.h"

struct timespec offset;
typedef struct custom_time_t{
  int s;
  int ms;
  int us;
  int ns;
}custom_time_t;

custom_time_t __get_relative_time(){
  custom_time_t out;
  struct timespec tempo;
  clock_gettime(CLOCK_REALTIME, &tempo);
  out.s  = (tempo.tv_sec - offset.tv_sec);
  out.ms  = (tempo.tv_nsec - offset.tv_nsec)/1000000;
  out.us  = ((tempo.tv_nsec - offset.tv_nsec)/1000)%1000;
  out.ns  = (tempo.tv_nsec - offset.tv_nsec)%1000;

  if(out.ns < 0){
    out.ns += 1000;
    out.us--;
  }
  if(out.us < 0){
    out.us += 1000;
    out.ms--;
  }
  if(out.ms < 0){
    out.ms += 1000;
    out.s--;
  }
  return out;
}

void __write_head(FILE* stream){
  const custom_time_t relative_time = __get_relative_time();
  fprintf(stdout, "[%.3d %.3d %.3d %.3d] : ", relative_time.s, relative_time.ms, relative_time.us, relative_time.ns);
}

void _write_log(const char* str){
  __write_head(stdout);
  fprintf(stdout, "%s\n", str);
  //fclose(stream);
}

void timing_init(){
  clock_gettime(CLOCK_REALTIME, &offset);
  _write_log("Init OK !");
}
/*
  @Paramters
  -p : period
  -n : normal WCET
  -a : abnormal WCET
  -d : deadline

  2 -
*/
int main (int argc, char **argv)
{
     long int random;
     srand(time(NULL));
     struct sched_attr attr;

     attr.size = sizeof(struct sched_attr);
     attr.sched_flags = 0;
     attr.sched_nice = 0;
     attr.sched_priority = 0;

     attr.sched_policy = SCHED_DEADLINE;
     attr.sched_runtime = 10 * 1000000;
     attr.sched_deadline = 15 * 1000000;
     attr.sched_period = 1000 * 1000000;

     if (sched_setattr(0, &attr, 0)) {
           perror("sched_setattr()");
           return 0;
     }

     timing_init();
     for(;;){
       _write_log("Loop starts ...");
       random = rand()%10000000;
       while(random--);
       _write_log("Loop ends ...");
       puts("============================");
       sched_yield();
     }

     return 0;
}