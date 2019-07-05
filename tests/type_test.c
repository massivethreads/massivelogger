#include "test_helper.h"

#include "mlog/mlog.h"
#include <string.h>

#define N 10

mlog_data_t g_md;

int g_a[N];

typedef struct mystruct {
  int v1;
  int v2[N];
  double v3;
} mystruct_t;

typedef int (*myfunc_t)();

int foo() {
  return 10;
}

void* decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1) {
  short          hd = MLOG_READ_ARG(&buf0, short);
  unsigned short hu = MLOG_READ_ARG(&buf0, unsigned short);
  int            d  = MLOG_READ_ARG(&buf0, int);
  unsigned int   u  = MLOG_READ_ARG(&buf0, unsigned int);
  long           ld = MLOG_READ_ARG(&buf0, long);
  unsigned long  lu = MLOG_READ_ARG(&buf0, unsigned long);
  float          f  = MLOG_READ_ARG(&buf0, float);
  double         lf = MLOG_READ_ARG(&buf0, double);
  char           c  = MLOG_READ_ARG(&buf0, char);
  char*          s  = MLOG_READ_ARG(&buf0, char*);

  fprintf(stream, "%d,%d,%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%c,%s\n", rank0, rank1, hd, hu, d, u, ld, lu, f, lf, c, s);

  int* a        = MLOG_READ_ARG(&buf1, int*);
  mystruct_t st = MLOG_READ_ARG(&buf1, mystruct_t);
  myfunc_t fp   = MLOG_READ_ARG(&buf1, myfunc_t);

  for (int i = 0; i < N; i++) {
    fprintf(stream, "%d,", a[i]);
  }

  fprintf(stream, "%d,", st.v1);
  for (int i = 0; i < N; i++) {
    fprintf(stream, "%d,", st.v2[i]);
  }
  fprintf(stream, "%f,", st.v3);

  fprintf(stream, "%d\n", (*fp)());

  return buf1;
}

int main() {
  mlog_init(&g_md, 1, 1024);

  char*  buf               = NULL;
  size_t buf_size          = 0;
  char*  expected_buf      = NULL;
  size_t expected_buf_size = 0;

  FILE* stream = open_memstream(&buf, &buf_size);
  ASSERT(stream != NULL, "Open Stream");

  FILE* expected_stream = open_memstream(&expected_buf, &expected_buf_size);
  ASSERT(expected_stream != NULL, "Open Stream");

  short          hd = -1;
  unsigned short hu = 1;
  int            d  = -2;
  unsigned int   u  = 2;
  long           ld = -3;
  unsigned long  lu = 3;
  float          f  = -0.25f;
  double         lf = 0.25;
  char           c  = 'c';
  char*          s  = "test";

  for (int i = 0; i < N; i++) {
    g_a[i] = i;
  }

  mystruct_t st;
  st.v1 = 100;
  for (int i = 0; i < N; i++) {
    st.v2[i] = -i;
  }
  st.v3 = 1000.0;

  // write
  void* x0 = MLOG_BEGIN(&g_md, 0, hd, hu, d, u, ld, lu, f, lf, c, s);
  MLOG_END(&g_md, 0, x0, decoder, g_a, st, foo);

  mlog_flush_all(&g_md, stream);

  // assert
  fprintf(expected_stream, "0,0,%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, c, s);
  for (int i = 0; i < N; i++) {
    fprintf(expected_stream, "%d,", g_a[i]);
  }
  fprintf(expected_stream, "%d,", st.v1);
  for (int i = 0; i < N; i++) {
    fprintf(expected_stream, "%d,", st.v2[i]);
  }
  fprintf(expected_stream, "%f,", st.v3);
  fprintf(expected_stream, "%d\n", foo());
  fflush(expected_stream);

  ASSERT(buf_size == expected_buf_size, "left:\n%d\nright:\n%d", buf_size, expected_buf_size);
  ASSERT(!strncmp(buf, expected_buf, buf_size), "left:\n%s\nright:\n%s", buf, expected_buf);

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
