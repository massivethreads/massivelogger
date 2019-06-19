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

typedef struct mlog_data {
  int       num_ranks;
  void**    begin_buffer;
  void**    end_buffer;
  void**    begin_buffer_0;
  void**    end_buffer_0;
}
mlog_data_t;

typedef int (*mlog_decoder_t)(FILE*, int, int, void*, void*);

/*
 * mlog_init
 */

static inline void mlog_init(mlog_data_t* md, int num_ranks) {
  md->num_ranks      = num_ranks;
  md->begin_buffer   = (void**)malloc(num_ranks * sizeof(void*));
  md->end_buffer     = (void**)malloc(num_ranks * sizeof(void*));
  md->begin_buffer_0 = (void**)malloc(num_ranks * sizeof(void*));
  md->end_buffer_0   = (void**)malloc(num_ranks * sizeof(void*));
  for (int rank = 0; rank < num_ranks; rank++) {
    md->begin_buffer[rank]   = (void*)malloc(MLOG_BUFFER_SIZE);
    md->end_buffer[rank]     = (void*)malloc(MLOG_BUFFER_SIZE);
    md->begin_buffer_0[rank] = md->begin_buffer[rank];
    md->end_buffer_0[rank]   = md->end_buffer[rank];
  }
}

/*
 * MLOG_BEGIN
 */

#define MLOG_BEGIN_WRITE_ARG(arg) MLOG_BEGIN_WRITE_ARG_(_mlog_rank, arg)
#define MLOG_BEGIN_WRITE_ARG_(rank, arg) \
  *((__typeof__(arg)*)(_mlog_md->begin_buffer[rank])) = arg; \
  _mlog_md->begin_buffer[rank] += sizeof(arg);

#define MLOG_BEGIN(md, rank, decoder, ...) \
  ((md)->begin_buffer[rank]); \
  { \
    mlog_data_t* _mlog_md = (md); \
    int _mlog_rank = (rank); \
    _mlog_write_decoder_to_begin_buffer(_mlog_md, _mlog_rank, (decoder)); \
    MLOG_FOREACH(MLOG_BEGIN_WRITE_ARG, __VA_ARGS__); \
  }

static inline void _mlog_write_decoder_to_begin_buffer(mlog_data_t* md, int rank, mlog_decoder_t decoder) {
  *((mlog_decoder_t*)(md->begin_buffer[rank])) = decoder;
  md->begin_buffer[rank] = ((char*)(md->begin_buffer[rank])) + sizeof(mlog_decoder_t);
}

/*
 * MLOG_END
 */

#define MLOG_END_WRITE_ARG(arg) MLOG_END_WRITE_ARG_(_mlog_rank, arg)
#define MLOG_END_WRITE_ARG_(rank, arg) \
  *((__typeof__(arg)*)(_mlog_md->end_buffer[rank])) = arg; \
  _mlog_md->end_buffer[rank] += sizeof(arg);

#define MLOG_END(md, rank, begin_ptr, ...) { \
    mlog_data_t* _mlog_md = (md); \
    int _mlog_rank = (rank); \
    _mlog_write_begin_ptr_to_end_buffer(_mlog_md, _mlog_rank, (begin_ptr)); \
    MLOG_FOREACH(MLOG_END_WRITE_ARG, __VA_ARGS__); \
  }

static inline void _mlog_write_begin_ptr_to_end_buffer(mlog_data_t* md, int rank, void* begin_ptr) {
  *((void**)md->end_buffer[rank]) = begin_ptr;
  md->end_buffer[rank] = ((char*)(md->end_buffer[rank])) + sizeof(void*);
}

/*
 * mlog_clear
 */

static inline void mlog_clear_begin_buffer(mlog_data_t* md, int rank) {
  md->begin_buffer[rank] = md->begin_buffer_0[rank];
}

static inline void mlog_clear_begin_buffer_all(mlog_data_t* md) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    mlog_clear_begin_buffer(md, rank);
  }
}

static inline void mlog_clear_end_buffer(mlog_data_t* md, int rank) {
  md->end_buffer[rank] = md->end_buffer_0[rank];
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

static inline int _mlog_get_rank_from_begin_ptr(mlog_data_t* md, void* begin_ptr) {
  for (int rank = 0; rank < md->num_ranks; rank++) {
    if (md->begin_buffer_0[rank] <= begin_ptr && begin_ptr < md->begin_buffer[rank]) {
      return rank;
    }
  }
  return -1;
}

static inline void mlog_flush(mlog_data_t* md, int rank, FILE* stream) {
  void* cur_end_buffer = md->end_buffer_0[rank];
  while (cur_end_buffer < md->end_buffer[rank]) {
    void*          begin_ptr = *((void**)cur_end_buffer);
    mlog_decoder_t decoder   = *((mlog_decoder_t*)begin_ptr);
    void*          buf0      = (char*)begin_ptr + sizeof(mlog_decoder_t);
    void*          buf1      = (char*)cur_end_buffer + sizeof(void*);
    int            rank0     = _mlog_get_rank_from_begin_ptr(md, begin_ptr);
    int            rank1     = rank;
    int            buf1_size = decoder(stream, rank0, rank1, buf0, buf1);
    cur_end_buffer = (char*)buf1 + buf1_size;
  }
  md->end_buffer[rank] = md->end_buffer_0[rank];
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
