# MassiveLogger

Lightweight Logging Library for Multi-Threading.

## Screenshot of Trace Viewer

![Screenshot of Trace Viewer](./examples/mandelbrot/mlog-example.png "A timeline trace plot of the Mandelbrot example parallelized by OpenMP (x: time, y: thread)")

## Getting Started
```sh
cd examples/mandelbrot
make
./mandelbrot
cd ../../
./run_viewer.bash examples/mandelbrot/mlog.txt
```

For more details about Mandelbrot example, see [examples/mandelbrot/README.md](examples/mandelbrot/README.md).

## Test
```sh
mkdir build
cd build
cmake ..
make
make check
```

## Low-level API

### mlog_data

```c
typedef struct mlog_data { /* implementation defined */ } mlog_data_t;
```

Log buffering structure for a whole MassiveLogger environment.

### mlog_init
```c
void mlog_init(mlog_data_t* md, int num_ranks, size_t buf_size);
```

Parameters:
* `md`        : Pointer to a `mlog_data_t` variable at global scope.
* `num_ranks` : The number of ranks (e.g., the number of workers/threads using MassiveLogger).
* `buf_size`  : The minimum unit of buffer memory size. The current implementation first allocates 2 * `buf_size` bytes of memory for each rank.

### MLOG_BEGIN
```c
void* MLOG_BEGIN(mlog_data_t* md, int rank, ...);
```

Parameters:
* `md`   : Pointer to a `mlog_data_t` variable at global scope.
* `rank` : e.g., worker ID or thread ID.
* `...`  : Arguments to record.

Return value:
* Pointer to the recorded data (`begin_ptr`). This should be passed to `MLOG_END` function.

### MLOG_END
```c
void MLOG_END(mlog_data_t* md, int rank, void* begin_ptr, void* (*decoder)(FILE*, int, int, void*, void*), ...);
```

Parameters:
* `md`        : Pointer to a `mlog_data_t` variable at global scope.
* `rank`      : e.g., worker ID or thread ID.
* `begin_ptr` : The return value of `MLOG_BEGIN` function.
* `decoder`   : Function pointer to a decorder function that transfers recorded data into formatted string. This is called when outputting recorded data to files. See below for more details.
* `...`       : Arguments to record.

`decoder` should be defined as follows.
```c
void* decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1);
```

Parameters:
* `stream` : File stream to write output.
* `rank0`  : Who calls `MLOG_BEGIN`.
* `rank1`  : Who calls `MLOG_END`.
* `buf0`   : Pointer to the beginning of the recorded arguments in `MLOG_BEGIN`.
* `buf1`   : Pointer to the beginning of the recorded arguments in `MLOG_END`.

Return value:
* Pointer to the end of the recorded arguments in `MLOG_END`. You can use `MLOG_READ_ARG` macro to read args, and when you have read all recorded args, `buf1` should be the return value.

### MLOG_PRINTF
```c
void MLOG_PRINTF(mlog_data_t* md, int rank, char* format, ...);
```

Parameters:
* `md`     : Pointer to a `mlog_data_t` variable at global scope.
* `rank`   : e.g., worker ID or thread ID.
* `format` : Format string (usually passed to `printf` function).
* `...`    : Arguments to record.

Note:
* Arguments passed to `MLOG_PRINTF` are not converted to string when `MLOG_PRINTF` is called.
  They are converted to string when `mlog_flush` is called.
* Due to its implementation, type is more strict than `printf`.
  With `printf`, types of arguments are automatically converted (e.g., float is converted to double), but with `MLOG_PRINTF`, they are not converted.
  Therefore you should explicitly specify types in `format` and to arguments (see `tests/printf_test.c`).
* This is different from the C standard, but you must use "%f" with float.
  If you want to record double value, you should use "%lf" specifier.
* "%n" is not supported.
* Currently variable field width `*` specifier (e.g., `"%.*s"`) is not supported.

### mlog_flush
```c
void mlog_flush(mlog_data_t* md, int rank, FILE* stream);
```

Parameters:
* `md`     : Pointer to a `mlog_data_t` variable at global scope.
* `rank`   : Logs in the end buffer of `rank` are flushed.
* `stream` : Logs are written to `stream`.

### mlog_flush_all
```c
void mlog_flush_all(mlog_data_t* md, FILE* stream);
```

Parameters:
* `md`     : Pointer to a `mlog_data_t` variable at global scope.
* `stream` : All logs are written to `stream`.

### MLOG_READ_ARG
```c
#define MLOG_READ_ARG(/* void** */ buf, type) /* ... */
```

Parameters:
* `buf` : Pointer to the buffer pointer.
          The buffer pointer is advanced by `sizeof(type)`.
* `type`: Type of the stored value.

Return value:
* Value loaded from buffer.

### mlog_warmup
```c
void mlog_warmup(mlog_data_t* md, int rank);
```

Write some values to entire buffers to avoid page faults while recording.

Parameters:
* `md`   : Pointer to a `mlog_data_t` variable at global scope.
* `rank` : e.g., worker ID or thread ID.

Note:
* You should be aware of NUMA effect; that is, if the memory allocation policy is `first-touch` policy, buffers are allocated to a NUMA node where `mlog_warmup()` is called. If you warm-up all threads' buffers in the main thread, all of buffers are allocated to a single NUMA node, which causes performance degradation in NUMA environment.

## High-level API

### mlog_begin_tl
```c
void* mlog_begin_tl(mlog_data_t* md, int rank);
```

`rank` and a timestamp (the return value of `mlog_clock_gettime_in_nsec()`) are recorded to the begin buffer.

Parameters:
* `md`   : Pointer to a `mlog_data_t` variable at global scope.
* `rank` : e.g., worker ID or thread ID.

Return value:
* A pointer to the recorded data (`begin_ptr`). This should be passed to `mlog_end_tl` function.

### mlog_end_tl
```c
void mlog_end_tl(mlog_data_t* md, int rank, void* begin_ptr, char* event_name);
```

`rank`, a timestamp (the return value of `mlog_clock_gettime_in_nsec()`), and `event_name` are recorded to the end buffer.

Parameters:
* `md`         : Pointer to a `mlog_data_t` variable at global scope.
* `rank`       : e.g., worker ID or thread ID.
* `begin_ptr`  : The return value of `mlog_begin_tl` function.
* `event_name` : Event name to be recorded.

## Functions for Time Measurement

### mlog_gettimeofday_in_usec
```c
uint64_t mlog_gettimeofday_in_usec();
```

Return the value of `gettimeofday` in micro seconds.

### mlog_clock_gettime_in_nsec
```c
uint64_t mlog_clock_gettime_in_nsec();
```

Return the value of `clock_gettime` in nano seconds.
`CLOCK_MONOTONIC` option is specified to `clock_gettime`.

### mlog_rdtsc
```c
uint64_t mlog_rdtsc();
```

Return the value of `rdtsc` instruction.

## Compilation Config
How to set flags:
```c
#define MLOG_XXX 1
#include <mlog/mlog.h>
```

### MLOG_DISABLE_CHECK_BUFFER_SIZE
Disable checking the size of begin and end buffer with every log call (default: 0).
Disabling the check may speed up the logging feature, but it can cause segmentation fault (*unsafe*).

### MLOG_DISABLE_REALLOC_BUFFER
Disable realloc of start and end buffer when buffers become full (default: 0).
When this flag is set and buffers become full, the execution is aborted without resizing buffers.
This prevents unconscious overheads with reallocation when measuring the performance.

## Viewer Config
* By default, the viewer is automatically terminated when the first web session is closed.
  We can make the server lifetime permanent by setting an environment variable `MLOG_VIEWER_ONESHOT=false`.

## Illustration of Buffers

`MLOG_BEGIN` and `MLOG_END`:
```
                    buf0
                     |
                     v
          -----------------------------------------------------------------------------------
rank0          |           |           |
           ... |   arg1    |   arg2    |            ...
begin_buf      |           |           |
          -----------------------------------------------------------------------------------
                     ^
                     |                      buf1
                     |                       |
                     |                       v
          -----------------------------------------------------------------------------------
rank1          |           |           |           |           |           |           |
           ... | begin_ptr |  decoder  |   arg1    |   arg2    |    ...    | begin_ptr | ...
end_buf        |           |           |           |           |           |           |
          -----------------------------------------------------------------------------------
```

`MLOG_PRINTF`:
```
          -----------------------------------------------------------------------------------
               |           |           |           |           |           |           |
end_buf    ... |   NULL    |  format   |   arg1    |   arg2    |    ...    |   NULL    | ...
               |           |           |           |           |           |           |
          -----------------------------------------------------------------------------------
```

## LICENSE

MassiveLogger is distributed under the 2-Clause BSD License. See [LICENSE](./LICENSE) for further information.
