#include <stdio.h>
#include <pthread.h>

/* #define MLOG_DISABLE_CHECK_BUFFER_SIZE 1 */
#define MLOG_DISABLE_REALLOC_BUFFER 1
#include "mlog/mlog.h"

#define WARMUP 1

mlog_data_t g_md;
pthread_barrier_t g_barrier;

typedef struct {
  int rank;
  int n_iter;
} thread_arg_t;

void* thread_fn(void* _arg) {
  thread_arg_t* arg = (thread_arg_t*)_arg;
  int rank = arg->rank;
  int n_iter = arg->n_iter;

#if WARMUP
  mlog_warmup(&g_md, rank);
#endif

  pthread_barrier_wait(&g_barrier);

  uint64_t t1 = mlog_clock_gettime_in_nsec();

  for (int i = 0; i < n_iter; i++) {
    void* x = mlog_begin_tl(&g_md, rank);
    mlog_end_tl(&g_md, rank, x, "event 0");
  }

  uint64_t t2 = mlog_clock_gettime_in_nsec();

  printf("[rank %d] total: %ld nsec, ave: %ld nsec\n", rank, t2 - t1, (t2 - t1) / n_iter);

  return NULL;
}

int main() {
  int n_threads = 4;
  int n_iter = 10000000;

  printf("Simple Benckmark for MassiveLogger\n"
         "  n_threads = %d\n"
         "  n_iter    = %d\n\n",
         n_threads, n_iter);

  mlog_init(&g_md, n_threads, (2 << 28));

  pthread_barrier_init(&g_barrier, NULL, n_threads);

  pthread_t threads[n_threads];
  thread_arg_t args[n_threads];
  for (int i = 0; i < n_threads; i++) {
    args[i].rank = i;
    args[i].n_iter = n_iter;
    pthread_create(&threads[i], NULL, &thread_fn, &args[i]);
  }

  for (int i = 0; i < n_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
