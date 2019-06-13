#pragma once
#ifndef MASSIVE_LOGGER_H_
#define MASSIVE_LOGGER_H_

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE (2 << 20)

int    g_mlog_num_ranks;
void** g_mlog_begin_buffer;
void** g_mlog_end_buffer;
void** g_mlog_begin_buffer_0;
void** g_mlog_end_buffer_0;

typedef int (*mlog_decoder_t)(FILE*, int, int, void*, void*);

/*
 * mlog_init
 */

void mlog_init(int num_ranks) {
  g_mlog_num_ranks      = num_ranks;
  g_mlog_begin_buffer   = (void**)malloc(num_ranks * sizeof(void*));
  g_mlog_end_buffer     = (void**)malloc(num_ranks * sizeof(void*));
  g_mlog_begin_buffer_0 = (void**)malloc(num_ranks * sizeof(void*));
  g_mlog_end_buffer_0   = (void**)malloc(num_ranks * sizeof(void*));
  for (int rank = 0; rank < num_ranks; rank++) {
    g_mlog_begin_buffer[rank]   = (void*)malloc(BUFFER_SIZE);
    g_mlog_end_buffer[rank]     = (void*)malloc(BUFFER_SIZE);
    g_mlog_begin_buffer_0[rank] = g_mlog_begin_buffer[rank];
    g_mlog_end_buffer_0[rank]   = g_mlog_end_buffer[rank];
  }
}

/*
 * MLOG_BEGIN
 */

#define MLOG_BEGIN_WRITE_ARG(arg) MLOG_BEGIN_WRITE_ARG_(_mlog_rank, arg)
#define MLOG_BEGIN_WRITE_ARG_(rank, arg) \
  *((typeof(arg)*)(g_mlog_begin_buffer[rank])) = arg; \
  g_mlog_begin_buffer[rank] += sizeof(arg);

#define MLOG_BEGIN(rank, decoder, ...) \
  g_mlog_begin_buffer[rank]; \
  { \
    int _mlog_rank = rank; \
    _mlog_write_decoder_to_begin_buffer(_mlog_rank, decoder); \
    MLOG_FOREACH(MLOG_BEGIN_WRITE_ARG, __VA_ARGS__); \
  }

static inline void _mlog_write_decoder_to_begin_buffer(int rank, mlog_decoder_t decoder) {
  *((mlog_decoder_t*)(g_mlog_begin_buffer[rank])) = decoder;
  g_mlog_begin_buffer[rank] += sizeof(mlog_decoder_t);
}

/*
 * MLOG_END
 */

#define MLOG_END_WRITE_ARG(arg) MLOG_END_WRITE_ARG_(_mlog_rank, arg)
#define MLOG_END_WRITE_ARG_(rank, arg) \
  *((typeof(arg)*)(g_mlog_end_buffer[rank])) = arg; \
  g_mlog_end_buffer[rank] += sizeof(arg);

#define MLOG_END(rank, begin_ptr, ...) { \
    int _mlog_rank = rank; \
    _mlog_write_begin_ptr_to_end_buffer(rank, begin_ptr); \
    MLOG_FOREACH(MLOG_END_WRITE_ARG, __VA_ARGS__); \
  }

static inline void _mlog_write_begin_ptr_to_end_buffer(int rank, void* begin_ptr) {
  *((void**)g_mlog_end_buffer[rank]) = begin_ptr;
  g_mlog_end_buffer[rank] += sizeof(void*);
}

/*
 * mlog_flush
 */

static inline int _mlog_get_rank_from_begin_ptr(void *begin_ptr) {
  for (int rank = 0; rank < g_mlog_num_ranks; rank++) {
    if (g_mlog_begin_buffer_0[rank] <= begin_ptr && begin_ptr < g_mlog_begin_buffer[rank]) {
      return rank;
    }
  }
  return -1;
}

void mlog_flush(int rank, FILE* stream) {
  void* cur_end_buffer = g_mlog_end_buffer_0[rank];
  while (cur_end_buffer < g_mlog_end_buffer[rank]) {
    void*          begin_ptr = *((void**)cur_end_buffer);
    mlog_decoder_t decoder   = *((mlog_decoder_t*)begin_ptr);
    void*          buf0      = begin_ptr + sizeof(mlog_decoder_t);
    void*          buf1      = cur_end_buffer + sizeof(void*);
    int            rank0     = _mlog_get_rank_from_begin_ptr(begin_ptr);
    int            rank1     = rank;
    int            buf1_size = decoder(stream, rank0, rank1, buf0, buf1);
    cur_end_buffer = buf1 + buf1_size;
  }
}

#endif /* MASSIVE_LOGGER_H_ */
