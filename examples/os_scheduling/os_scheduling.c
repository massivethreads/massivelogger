#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "mlog/mlog.h"

typedef struct {
  int      rank;
  uint64_t num;
} fn_args_t;

mlog_data_t g_md;

void* my_decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1) {
  (void)rank1;
  uint64_t t0  = MLOG_READ_ARG(&buf0, uint64_t);
  int      cpu = MLOG_READ_ARG(&buf0, int);
  uint64_t t1  = MLOG_READ_ARG(&buf1, uint64_t);

  fprintf(stream, "%d,%lu,%d,%lu,thread%03d\n", cpu, t0, cpu, t1, rank0);
  return buf1;
}

void* thread_fn(void* args) {
  fn_args_t* fn_args = (fn_args_t*)args;
  int      rank = fn_args->rank;
  uint64_t num  = fn_args->num;

  uint64_t t0 = mlog_clock_gettime_in_nsec();
  void* bp = MLOG_BEGIN(&g_md, rank, t0, sched_getcpu());
  for (uint64_t i = 0; i < num; i++) {
    uint64_t t1 = mlog_clock_gettime_in_nsec();
    if (t1 - t0 > 100000) {
      MLOG_END(&g_md, rank, bp, my_decoder, t0);
      bp = MLOG_BEGIN(&g_md, rank, t1, sched_getcpu());
    }
    t0 = t1;
  }
  MLOG_END(&g_md, rank, bp, my_decoder, t0);

  return NULL;
}

void output_mlog() {
  FILE *fp = fopen("mlog.txt", "w");
  mlog_flush_all(&g_md, fp);
  fclose(fp);
}

int main(int argc, char* argv[]) {
  int      num_threads = 100;
  uint64_t num_iter    = 10000000;

  int opt;
  while ((opt = getopt(argc, argv, "t:i:h")) != EOF) {
    switch (opt) {
      case 't':
        num_threads = atoi(optarg);
        break;
      case 'i':
        num_iter = atol(optarg);
        break;
      case 'h':
      default:
        printf("Usage: ./os_scheduling -t <num_threads> -i <num_iter>\n");
        exit(1);
    }
  }

  mlog_init(&g_md, num_threads, (2 << 20));

  pthread_t* pthreads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
  fn_args_t* fn_args  = (fn_args_t*)malloc(sizeof(fn_args_t) * num_threads);

  printf("record started...\n");

  for (int i = 0; i < num_threads; i++) {
    fn_args[i].rank = i;
    fn_args[i].num  = num_iter;
    pthread_create(&pthreads[i], 0, thread_fn, &fn_args[i]);
  }

  for (int i = 0; i < num_threads; i++) {
    pthread_join(pthreads[i], NULL);
  }

  printf("success!\n");

  free(pthreads);
  free(fn_args);

  output_mlog();

  return 0;
}
