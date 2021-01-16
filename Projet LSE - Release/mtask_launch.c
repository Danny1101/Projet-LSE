#define _GNU_SOURCE

#include "reg.h"

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <syscall.h>

#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

#include "log/log.h"

#include "log/log.h"
#include "syscalls/syscalls.h"
#include "trace/trace.h"
#define US_TO_NS(X) ((X)*1000)
//=======================================================
const char* usage = "Usage: mtask_launch [args] [executable] [options executable]\n" \
"Lance une tache supervisee.\n" \
"\n" \
"Les arguments obligatoires sont :\n" \
"  -p, --period           Période en microsecondes\n" \
"  -d, --deadline         Echéance en microsecondes\n" \
"  -n, --normal_wcet      Pire temps d'exécution normal en microsecondes\n" \
"  -a, --abnormal_wcet    Pire temps d'exécution anormal en microsecondes\n";
//=======================================================
//=======================================================
static void __error_handle(int cond, const char* str);
static int __parse_attr(int argc, char** argv, struct sched_deadline_params_t*);
static char** __create_argv_array(int argc, char** argv);
static void __clear_argv_array(int argc, char** argv);
//=======================================================
int main(int argc, char** argv){
  volatile pid_t cpid = 0;
  int exec_index;
  char** child_argv = NULL;
  int child_argc;
  struct sched_deadline_params_t sched_deadline_params;
  struct raw_log_t raw_log;

  //================== Set task attr ====================
  exec_index = 1; //__parse_attr(argc, argv, &sched_deadline_params);
  child_argv = __create_argv_array(child_argc=(argc - exec_index), argv + exec_index);
  log_init(&raw_log);
  //======================== FORK =======================
  cpid = fork();
  if(cpid > 0){
    //==========================================================================
    // Parent code
    raw_log.pid = cpid;
    trace_child(cpid, &raw_log, &sched_deadline_params);
    //==========================================================================
  }else if(!cpid){
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    return execv(child_argv[0], child_argv);
  }


  //=====================================================
main_dies:
  __clear_argv_array(child_argc, child_argv);
  log_free(&raw_log);
  return EXIT_SUCCESS;
}
//==================================================
static char** __create_argv_array(int argc, char** argv){
  char** out = NULL;
  out = (char**)calloc(argc+1, sizeof(char*));
  for(int i=0; i<argc; i++){

    out[i] = (char*) calloc(strlen(argv[i]) + 1, sizeof(char));
    strcpy(out[i], argv[i]);

  }
  return out;
}
static void __clear_argv_array(int argc, char** argv){
  if(!argv)return;
  for(int i=0; i < argc; free(argv[i++]));
  free(argv);
  argv = NULL;
}
//==================================================
static void __error_handle(int cond, const char* str){
  if(cond){
    fputs(str, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
  }
}
static int __parse_attr(int argc, char** argv, struct sched_deadline_params_t* params){
  int fill_values = 0;
  int index = -1;

  __error_handle( argc < 3 , usage);

  // Capture arguments and store them in {params}
  for (size_t i = 1; i < argc; i++) {
    if( (!strcmp(argv[i], "-p")) || (!strcmp(argv[i], "--period")) ){
      __error_handle(sscanf(argv[++i], "%lld", &params->period_us) != 1, "Impossible de recueillir la valeur de la période...");
      fill_values |= 1;
    }

    else if( (!strcmp(argv[i], "-d")) || (!strcmp(argv[i], "--deadline")) ){
      __error_handle(sscanf(argv[++i], "%lld", &params->deadline_us) != 1, "Impossible de recueillir la valeur de l'échéance ...");
      fill_values |= 2;
    }

    else if( (!strcmp(argv[i], "-n")) || (!strcmp(argv[i], "--normal_wcet")) ){
      __error_handle(sscanf(argv[++i], "%lld", &params->normal_wcet_us) != 1, "Impossible de recueillir la valeur du pire temps d'exécution normal ...");
      fill_values |= 4;
    }

    else if( (!strcmp(argv[i], "-a")) || (!strcmp(argv[i], "--abnormal_wcet")) ){
      __error_handle(sscanf(argv[++i], "%lld", &params->abnormal_wcet_us) != 1, "Impossible de recueillir la valeur du pire temps d'exécution anormal ...");
      fill_values |= 8;
    }
    else{
      index = i;
      break;
    }
  }

  __error_handle((index == -1), "Erreur, executable non trouvé");
  __error_handle(!(fill_values & (1)), "Erreur, période non définie");

  if(!(fill_values & (1<<1))){
    fprintf(stderr, "Attention, l'échéance n'est pas définie, elle prendra la valeur de la période (%lld us)\n", params->deadline_us = params->period_us);
  }else if(params->deadline_us > params->period_us){
    fprintf(stderr, "Attention, l'échéance est supérieure à la période, elle sera redéfinie sur la valeur de la période (%lld us)\n", params->deadline_us = params->period_us);
  }

  if(!(fill_values & (1<<2))){
    fprintf(stderr, "Attention, le temps d'exécution normal n'est pas défini, il prendra la valeur de l'échéance (%lld us)\n", params->normal_wcet_us = params->deadline_us);
  }else if(params->normal_wcet_us > params->deadline_us){
    fprintf(stderr, "Attention, le temps d'exécution normal est supérieure à l'échéance, il sera redéfini sur la valeur de l'échéance (%lld us)\n", params->normal_wcet_us = params->deadline_us);
  }

  if(!(fill_values & (1<<3))){
    fprintf(stderr, "Attention, le temps d'exécution anormal n'est pas défini, il prendra la valeur du temps d'exécution normal (%lld us)\n", params->abnormal_wcet_us = params->normal_wcet_us);
  }else if(params->normal_wcet_us > params->deadline_us){
    fprintf(stderr, "Attention, le temps d'exécution anormal est supérieure à l'échéance, il sera redéfini sur la valeur de l'échéance (%lld us)\n", params->abnormal_wcet_us = params->deadline_us);
  }


  return index;
}
//==================================================
//==================================================
