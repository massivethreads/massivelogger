#include "test_helper.h"

#include "mlog/mlog.h"

mlog_data_t g_md;

int foo() {
  return 10;
}

int main() {
  mlog_init(&g_md, 3, 1024);

  char*  buf               = NULL;
  size_t buf_size          = 0;
  char*  expected_buf      = NULL;
  size_t expected_buf_size = 0;

  FILE* stream = open_memstream(&buf, &buf_size);
  ASSERT(stream != NULL, "Open Stream");

  FILE* expected_stream = open_memstream(&expected_buf, &expected_buf_size);
  ASSERT(expected_stream != NULL, "Open Stream");

  int16_t     hd = -1;
  uint16_t    hu = 1;
  int32_t     d  = -20;
  uint32_t    u  = 20;
  int64_t     ld = -300;
  uint64_t    lu = 300;
  float       f  = -0.25f;
  double      lf = 0.25;
  long double Lf = 0.33333;
  char        c  = 'c';
  const char* s  = "test";

  MLOG_PRINTF(&g_md, 0, "%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%Lf,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hi,%ho,%i,%o,%li,%lo,%F,%lF,%LF,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hd,%hx,%d,%x,%ld,%lx,%e,%le,%Le,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hd,%hX,%d,%X,%ld,%lX,%E,%lE,%LE,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hd,%hu,%d,%u,%ld,%lu,%g,%lg,%Lg,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hd,%hu,%d,%u,%ld,%lu,%G,%lG,%LG,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  MLOG_PRINTF(&g_md, 0, "%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%Lf,%c,%s\n", (short)-1, (unsigned short)1, -20, 20u, -300l, 300lu, -0.25f, 0.25, 0.33333l, (char)'c', "test");
  MLOG_PRINTF(&g_md, 1, "%p (main function pointer)\n", main);
  MLOG_PRINTF(&g_md, 1, "%p (pointer to int d)\n", &d);
  MLOG_PRINTF(&g_md, 1, "[%10s]\n[%-10s]\n", "Hello", "Hello");
  MLOG_PRINTF(&g_md, 1, "%+05d %+05d %.2s\n", foo(), d, s);
  MLOG_PRINTF(&g_md, 1, "% d %d %.2s\n", foo(), d, s);
  MLOG_PRINTF(&g_md, 2, "Loading ...\n");
  MLOG_PRINTF(&g_md, 2, "%%%d%%%%%s%c", 32, s, c);
  MLOG_PRINTF(&g_md, 2, "\n");
  MLOG_PRINTF(&g_md, 2, "");
  mlog_flush_all(&g_md, stream);

  // assert
  fprintf(expected_stream, "%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%Lf,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hi,%ho,%i,%o,%li,%lo,%F,%lF,%LF,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hd,%hx,%d,%x,%ld,%lx,%e,%le,%Le,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hd,%hX,%d,%X,%ld,%lX,%E,%lE,%LE,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hd,%hu,%d,%u,%ld,%lu,%g,%lg,%Lg,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hd,%hu,%d,%u,%ld,%lu,%G,%lG,%LG,%c,%s\n", hd, hu, d, u, ld, lu, f, lf, Lf, c, s);
  fprintf(expected_stream, "%hd,%hu,%d,%u,%ld,%lu,%f,%lf,%Lf,%c,%s\n", (short)-1, (unsigned short)1, -20, 20u, -300l, 300lu, -0.25f, 0.25, 0.33333l, (char)'c', "test");
  fprintf(expected_stream, "%p (main function pointer)\n", main);
  fprintf(expected_stream, "%p (pointer to int d)\n", &d);
  fprintf(expected_stream, "[%10s]\n[%-10s]\n", "Hello", "Hello");
  fprintf(expected_stream, "%+05d %+05d %.2s\n", foo(), d, s);
  fprintf(expected_stream, "% d %d %.2s\n", foo(), d, s);
  fprintf(expected_stream, "Loading ...\n");
  fprintf(expected_stream, "%%%d%%%%%s%c", 32, s, c);
  fprintf(expected_stream, "\n");
  fflush(expected_stream);

  /* ASSERT(buf_size == expected_buf_size, "left:\n%d\nright:\n%d", buf_size, expected_buf_size); */
  ASSERT(!strncmp(buf, expected_buf, buf_size), "left:\n%s\nright:\n%s", buf, expected_buf);

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
