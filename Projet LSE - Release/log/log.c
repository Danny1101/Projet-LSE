#include "log.h"

//==============================================================================
int fifo_log;
//==============================================================================
void log_init(struct raw_log_t* log){
  //mkfifo(LOG_FIFO_PATH, 0666);
  fifo_log = open(LOG_FIFO_PATH, O_WRONLY);
}
//==============================================================================
void log_offset_mark(struct raw_log_t* log){
  clock_gettime(CLOCK_REALTIME, &log->offset);
  printf("Offset set");
}
//==============================================================================
void log_preloop_mark(struct raw_log_t* log){
  clock_gettime(CLOCK_REALTIME, &log->preloop);
}
//==============================================================================
void log_postloop_mark(struct raw_log_t* log){
  clock_gettime(CLOCK_REALTIME, &log->postloop);
  write(fifo_log, log, sizeof(struct raw_log_t));
}
//==============================================================================
void log_free(struct raw_log_t* log){
  close(fifo_log);
}
