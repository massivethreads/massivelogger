#pragma once
#ifndef MLOG_MLOG_H_
#define MLOG_MLOG_H_

#include "mlog/base.h"
#include "mlog/time.h"

/*
 * High level functions
 */

int mlog_default_decoder_tl(FILE* stream, int rank0, int rank1, void* buf0, void* buf1) {
  void* buf1_0 = buf1;

  uint64_t t0         = MLOG_READ_ARG(&buf0, uint64_t);

  uint64_t t1         = MLOG_READ_ARG(&buf1, uint64_t);
  char*    event_name = MLOG_READ_ARG(&buf1, char*);

  fprintf(stream, "%d,%ld,%d,%ld,%s\n", rank0, t0, rank1, t1, event_name);

  return (int)((char*)buf1 - (char*)buf1_0);
}

static inline void* mlog_begin_tl(mlog_data_t* md, int rank) {
  void* ret = MLOG_BEGIN(md, rank, mlog_clock_gettime_in_nsec());
  return ret;
}

static inline void mlog_end_tl(mlog_data_t* md, int rank, void* begin_ptr, char* event_name) {
  MLOG_END(md, rank, begin_ptr, mlog_default_decoder_tl, mlog_clock_gettime_in_nsec(), event_name);
}

#endif /* MLOG_MLOG_H_ */

/* vim: set ts=2 sw=2 tw=0: */
