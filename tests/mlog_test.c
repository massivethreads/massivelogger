#include "test_helper.h"

#include "mlog/mlog.h"
#include <string.h>

mlog_data_t g_md;

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
  mlog_init(&g_md, 3);

  char*  buf               = NULL;
  size_t buf_size          = 0;
  char*  expected_buf      = NULL;
  size_t expected_buf_size = 0;

  FILE* stream = open_memstream(&buf, &buf_size);
  ASSERT(stream != NULL, "Open Stream");

  FILE* expected_stream = open_memstream(&expected_buf, &expected_buf_size);
  ASSERT(expected_stream != NULL, "Open Stream");

  int      rank0[] = {0       , 1       , 0       , 2       };
  int      rank1[] = {0       , 1       , 1       , 0       };
  uint64_t    t0[] = {12345ULL, 34567ULL, 45678ULL, 60000ULL};
  uint64_t    t1[] = {23456ULL, 67890ULL, 56789ULL, 99999ULL};
  int         i0[] = {1       , 2       , 3       , 4       };
  double      d0[] = {2.1     , 3.4     , 8.8     , 0.0     };
  float       f1[] = {3.3f    , -4.3f   , 0.0f    , 4.31f   };
  int         i1[] = {-1      , -2      , -3      , -4      };

  int order[] = {0, 3, 2, 1};
  for (int i = 0; i < 4; i++) {
    int idx = order[i];
    fprintf(expected_stream, "rank0: %d, rank1: %d, t0: %ld, t1: %ld, i0: %d, d0: %f, f1: %lf, i1: %d\n", rank0[idx], rank1[idx], t0[idx], t1[idx], i0[idx], d0[idx], f1[idx], i1[idx]);
  }
  fflush(expected_stream);

  void* x0 = MLOG_BEGIN(&g_md, rank0[0], decoder, t0[0], i0[0], d0[0]); // ---
                                                                 //   |
  MLOG_END(&g_md, rank1[0], x0, t1[0], f1[0], i1[0]);                   // <--
                                                                 //
  void* x1 = MLOG_BEGIN(&g_md, rank0[1], decoder, t0[1], i0[1], d0[1]); // ------
                                                                 //      |
  void* x2 = MLOG_BEGIN(&g_md, rank0[2], decoder, t0[2], i0[2], d0[2]); // ---  |
                                                                 //   |  |
  MLOG_END(&g_md, rank1[2], x2, t1[2], f1[2], i1[2]);                   // <--  |
                                                                 //      |
  void* x3 = MLOG_BEGIN(&g_md, rank0[3], decoder, t0[3], i0[3], d0[3]); // -----+---
                                                                 //      |  |
  MLOG_END(&g_md, rank1[1], x1, t1[1], f1[1], i1[1]);                   // <-----  |
                                                                 //         |
  MLOG_END(&g_md, rank1[3], x3, t1[3], f1[3], i1[3]);                   // <--------

  mlog_flush_all(&g_md, stream);

  ASSERT(buf_size == expected_buf_size, "left:\n%d\nright:\n%d", buf_size, expected_buf_size);
  ASSERT(!strncmp(buf, expected_buf, buf_size), "left:\n%s\nright:\n%s", buf, expected_buf);

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
