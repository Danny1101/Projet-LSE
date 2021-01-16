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
#include <sys/time.h>
 #include <sys/resource.h>

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
*/
int main (int argc, char **argv)
{
     long int random;
     srand(time(NULL));
     struct sched_attr attr;
     struct rusage prev, next;
     uint64_t utime, stime;
     struct timespec* t_sleep;

     int iteration=0;

     attr.size = sizeof(struct sched_attr);
     attr.sched_flags = 0;
     attr.sched_nice = 0;
     attr.sched_priority = 0;

     attr.sched_policy = SCHED_DEADLINE;
     attr.sched_runtime = 10 * 1000000;
     attr.sched_deadline = 15 * 1000000;
     attr.sched_period = 1000 * 1000000;

     /*
     if (sched_setattr(0, &attr, 0)) {
           perror("sched_setattr()");
           return 0;
     }
     */

     timing_init();
     getrusage(RUSAGE_SELF, &prev);

     for(;;){
       _write_log("Loop starts ...");
       random = rand()%200000;
       while(random--);
       getrusage(RUSAGE_SELF, &next);

       utime = (next.ru_utime.tv_sec - prev.ru_utime.tv_sec)*1000000 + next.ru_utime.tv_usec-prev.ru_utime.tv_usec;
       stime = (next.ru_stime.tv_sec - prev.ru_stime.tv_sec)*1000000 + next.ru_stime.tv_usec-prev.ru_stime.tv_usec;

       printf("user cpu time %lld\n", utime);
       printf("kernel cpu time %lld\n", stime);

       t_sleep = (struct timespec*)malloc(sizeof(struct timespec));
       printf("arg1 = %p\n",t_sleep);

       t_sleep->tv_sec = 0;
       t_sleep->tv_nsec = 500*1000*1000;

       _write_log("Loop ends ...");
       printf("============ %d =============\n", iteration++);
       prev = next;
       nanosleep(t_sleep, NULL);
       free(t_sleep);
     }

     return 0;
}
