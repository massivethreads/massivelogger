#include "test_helper.h"

#include "mlog/mlog.h"

mlog_data_t g_md;

int main() {
  mlog_init(&g_md, 3, 1024);

  char*  buf      = NULL;
  size_t buf_size = 0;

  FILE* stream = open_memstream(&buf, &buf_size);
  ASSERT(stream != NULL, "Open Stream");

  int rank0[] = {0, 1, 2, 0, 2};
  int rank1[] = {0, 1, 0, 1, 2};

  void* x0 = mlog_begin_tl(&g_md, rank0[0]); // event 0
  mlog_end_tl(&g_md, rank1[0], x0, "event 0");
  void* x1 = mlog_begin_tl(&g_md, rank0[1]); // event 1
  void* x2 = mlog_begin_tl(&g_md, rank0[2]); // event 2
  void* x3 = mlog_begin_tl(&g_md, rank0[3]); // event 3
  mlog_end_tl(&g_md, rank1[3], x3, "event 3");
  mlog_end_tl(&g_md, rank1[2], x2, "event 2");
  void* x4 = mlog_begin_tl(&g_md, rank0[4]); // event 4
  mlog_end_tl(&g_md, rank1[1], x1, "event 1");
  mlog_end_tl(&g_md, rank1[4], x4, "event 4");

  mlog_flush_all(&g_md, stream);

  // assert
  int r0, r1, event_id;
  uint64_t t0, t1;
  int n;
  int expected_order[] = {0, 2, 3, 1, 4};
  int i = 0;
  while (sscanf(buf, "%d,%ld,%d,%ld,event %d%n", &r0, &t0, &r1, &t1, &event_id, &n) == 5) {
    buf += n;
    int e = expected_order[i];
    ASSERT(r0 == rank0[e], "rank0: %d\nexpected: %d\n", r0, rank0[e]);
    ASSERT(r1 == rank1[e], "rank1: %d\nexpected: %d\n", r1, rank1[e]);
    ASSERT(t0 <= t1, "t0 (%ld) > t1 (%ld)\n", t0, t1);
    ASSERT(event_id == e, "event_id: %d\nexpected: %d\n", event_id, e);
    i++;
  }

  return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
