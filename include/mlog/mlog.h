#pragma once
#ifndef MLOG_MLOG_H_
#define MLOG_MLOG_H_

#include "mlog/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MLOG_BUFFER_SIZE (2 << 20)

typedef struct mlog_buffer {
  void*   first;
  void*   last;
}
mlog_buffer_t;

typedef struct mlog_data {
  int               num_ranks;
  mlog_buffer_t*    begin_buf;
  mlog_buffer_t*    end_buf;
}
mlog_data_t;

typedef int (*mlog_decoder_t)(FILE*, int, int, void*, void*);

/*
 * mlog_init
 */

static inline void mlog_buffer_init(mlog_buffer_t* buf) {
  buf->first = (void*)malloc(MLOG_BUFFER_SIZE);
  buf->last  = buf->first;
};

static inline void mlog_init(mlog_data_t* md, int num_ranks) {
  md->num_ranks = num_ranks;
  md->begin_buf = (mlog_buffer_t*)malloc(num_ranks * sizeof(mlog_buffer_t));
  md->end_buf   = (mlog_buffer_t*)malloc(num_ranks * sizeof(mlog_buffer_t));
  for (int rank = 0; rank < num_ranks; rank++) {
    mlog_buffer_init(&md->begin_buf[rank]);
    mlog_buffer_init(&md->end_buf[rank]);
  }
}

/*
 * MLOG_BEGIN
 */

#define MLOG_BUFFER_WRITE_VALUE(buf, arg) \
  *((__typeof__(arg)*)((buf)->last)) = (arg); \
  (buf)->last = ((char*)(buf)->last) + sizeof(arg);

#define MLOG_BEGIN_WRITE_ARG(arg) MLOG_BUFFER_WRITE_VALUE(_mlog_buf, arg)

#define MLOG_BEGIN(md, rank, ...) \
  ((md)->begin_buf[rank].last); \
  { \
    mlog_buffer_t* _mlog_buf = &((md)->begin_buf[rank]); \
    MLOG_FOREACH(MLOG_BEGIN_WRITE_ARG, __VA_ARGS__); \
  }

/*
 * MLOG_END
 */

#define MLOG_END_WRITE_ARG(arg) MLOG_BUFFER_WRITE_VALUE(_mlog_buf, arg)

#define MLOG_END(md, rank, begin_ptr, decoder, ...) { \
    mlog_buffer_t* _mlog_buf = &((md)->end_buf[rank]); \
    _mlog_write_begin_ptr_to_end_buffer(_mlog_buf, (begin_ptr)); \
    _mlog_write_decoder_to_end_buffer(_mlog_buf, (decoder)); \
    MLOG_FOREACH(MLOG_END_WRITE_ARG, __VA_ARGS__); \
  }

static inline void _mlog_write_begin_ptr_to_end_buffer(mlog_buffer_t* end_buf, void* begin_ptr) {
  MLOG_BUFFER_WRITE_VALUE(end_buf, begin_ptr);
}

static inline void _mlog_write_decoder_to_end_buffer(mlog_buffer_t* end_buf, mlog_decoder_t decoder) {
  MLOG_BUFFER_WRITE_VALUE(end_buf, decoder);
}

/*
 * MLOG_READ_ARG
 */

#define MLOG_READ_ARG(buf, type) \
  (*(buf) = (type*)*(buf)+1, *(((type*)*(buf))-1))

/*
 * mlog_clear
 */

static inline void mlog_clear_buffer(mlog_buffer_t* buf) {
  buf->last = buf->first;
}

static inline void mlog_clear_begin_buffer(mlog_data_t* md, int rank) {
  mlog_clear_buffer(&md->begin_buf[rank]);
}

static inline void mlog_clear_begin_buffer_all(mlog_data_t* md) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    mlog_clear_begin_buffer(md, rank);
  }
}

static inline void mlog_clear_end_buffer(mlog_data_t* md, int rank) {
  mlog_clear_buffer(&md->end_buf[rank]);
}

static inline void mlog_clear_end_buffer_all(mlog_data_t* md) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    mlog_clear_end_buffer(md, rank);
  }
}

static inline void mlog_clear(mlog_data_t* md, int rank) {
  mlog_clear_begin_buffer(md, rank);
  mlog_clear_end_buffer(md, rank);
}

static inline void mlog_clear_all(mlog_data_t* md) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    mlog_clear(md, rank);
  }
}

/*
 * mlog_flush
 */

static inline int _mlog_buffer_includes(mlog_buffer_t* buf, void* p) {
  return buf->first <= p && p < buf->last;
}

static inline int _mlog_get_rank_from_begin_ptr(mlog_data_t* md, void* begin_ptr) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    if (_mlog_buffer_includes(&md->begin_buf[rank], begin_ptr)) {
      return rank;
    }
  }
  return -1;
}

static inline void mlog_flush(mlog_data_t* md, int rank, FILE* stream) {
  void* cur_end_buffer = md->end_buf[rank].first;
  while (cur_end_buffer < md->end_buf[rank].last) {
    void*          begin_ptr = MLOG_READ_ARG(&cur_end_buffer, void*);
    mlog_decoder_t decoder   = MLOG_READ_ARG(&cur_end_buffer, mlog_decoder_t);
    void*          buf0      = begin_ptr;
    void*          buf1      = cur_end_buffer;
    int            rank0     = _mlog_get_rank_from_begin_ptr(md, begin_ptr);
    int            rank1     = rank;
    int            buf1_size = decoder(stream, rank0, rank1, buf0, buf1);
    cur_end_buffer = (char*)buf1 + buf1_size;
  }
  mlog_clear_end_buffer(md, rank);
  fflush(stream);
}

static inline void mlog_flush_all(mlog_data_t* md, FILE* stream) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    mlog_flush(md, rank, stream);
  }
  mlog_clear_begin_buffer_all(md);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* MLOG_MLOG_H_ */
/* vim: set ts=2 sw=2 tw=0: */
