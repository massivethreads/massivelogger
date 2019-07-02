#include "test_helper.h"

// If you commentout this flag, you can be sure that realloc is called and fails.
// #define MLOG_DISABLE_REALLOC_BUFFER 1
#include "mlog/mlog.h"

mlog_data_t g_md;

int main() {
  mlog_init(&g_md, 2, 1024);

  char*  buf      = NULL;
  size_t buf_size = 0;

  FILE* stream = open_memstream(&buf, &buf_size);
  ASSERT(stream != NULL, "Open Stream");

  size_t end_buf_size_per_write = sizeof(void*) + sizeof(mlog_decoder_t) + sizeof(uint64_t) + sizeof(char*); // 32 bytes

  int n_iter = 10;
  int n_log  = 5000;

  for (int it = 0; it < n_iter; it++) {
    for (int i = 0; i < n_log; i++) {
      void* x = mlog_begin_tl(&g_md, 0);
      mlog_end_tl(&g_md, 1, x, "event 0");

      ASSERT((char*)g_md.end_buf[1].last - (char*)g_md.end_buf[1].first == (int)end_buf_size_per_write * (i + 1),
          "end_buf assert failed (i = %d).\nleft: %ld\nright: %ld\n",
          i,
          (char*)g_md.end_buf[1].last - (char*)g_md.end_buf[1].first,
          end_buf_size_per_write * (i + 1));
    }

    mlog_flush_all(&g_md, stream);
  }

  // assert
  int r0, r1, event_id;
  uint64_t t0, t1;
  int n;
  int i = 0;
  while (sscanf(buf, "%d,%ld,%d,%ld,event %d%n", &r0, &t0, &r1, &t1, &event_id, &n) == 5) {
    buf += n;
    ASSERT(r0 == 0, "rank0: %d\nexpected: %d\n", r0, 0);
    ASSERT(r1 == 1, "rank1: %d\nexpected: %d\n", r1, 1);
    ASSERT(t0 <= t1, "t0 (%ld) > t1 (%ld)\n", t0, t1);
    ASSERT(event_id == 0, "event_id: %d\nexpected: %d\n", event_id, 0);
    i++;
  }

  ASSERT(i == n_iter * n_log, "# of read: %d\nexpected: %d\n", i, n_iter * n_log);

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
