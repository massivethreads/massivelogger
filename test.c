#include "massive_logger.h"

#include <sys/time.h>

static inline uint64_t get_time_in_usec() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * (uint64_t)1000 * 1000 + tv.tv_usec;
}

int decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1) {
  void* buf1_0 = buf1;

  uint64_t t0 = *((uint64_t*)buf0);
  buf0 += sizeof(uint64_t);
  int i0 = *((int*)buf0);
  buf0 += sizeof(int);
  double d0 = *((double*)buf0);
  buf0 += sizeof(double);

  uint64_t t1 = *((uint64_t*)buf1);
  buf1 += sizeof(uint64_t);
  float f1 = *((float*)buf1);
  buf1 += sizeof(float);
  int i1 = *((int*)buf1);
  buf1 += sizeof(int);

  fprintf(stream, "rank0: %d, rank1: %d, t0: %ld, t1: %ld, i0: %d, d0: %f, f1: %lf, i1: %d\n", rank0, rank1, t0, t1, i0, d0, f1, i1);

  return buf1 - buf1_0;
}

int main() {
  mlog_init(2);

  for (int i = 0; i < 3; i++) {
    void* x1 = MLOG_BEGIN(1, decoder, get_time_in_usec(), 1, 2.1);

    MLOG_END(1, x1, get_time_in_usec(), 3.1f, 1);

    void* x2 = MLOG_BEGIN(1, decoder, get_time_in_usec(), 2, 1.1);

    void* x3 = MLOG_BEGIN(1, decoder, get_time_in_usec(), 3, 1.1);

    void* x4 = MLOG_BEGIN(0, decoder, get_time_in_usec(), 4, 1.0);

    MLOG_END(1, x3, get_time_in_usec(), 2.99f, 3);

    MLOG_END(0, x2, get_time_in_usec(), 2.99f, 2);

    MLOG_END(0, x4, get_time_in_usec(), -33.3f, 4);

    mlog_flush_all(stderr);
  }

  return 0;
}
