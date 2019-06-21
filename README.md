# MassiveLogger

Lightweight Logging Library for Multi-Threading.

## Test
```sh
mkdir build
cd build
cmake ..
make
make check
```

## API

### mlog_data

```c
typedef struct mlog_data { /* implementation defined */ } mlog_data_t;
```

### mlog_init
```c
void mlog_init(mlog_data_t* md, int num_ranks);
```

Parameters:
* `md`        : Global log data for MassiveLogger.
* `num_ranks` : The number of ranks (e.g., workers or threads) who use MassiveLogger.

### MLOG_BEGIN
```c
void* MLOG_BEGIN(mlog_data_t* md, int rank, ...);
```

Parameters:
* `md`      : Global log data for MassiveLogger.
* `rank`    : e.g., worker ID or thread ID.
* `...`     : Arguments to record.

Return value:
* A pointer to the recorded data (`begin_ptr`). This should be passed to `MLOG_END` function.

### MLOG_END
```c
void MLOG_END(mlog_data_t* md, int rank, void* begin_ptr, int (*decoder)(FILE*, int, int, void*, void*), ...);
```

Parameters:
* `md`        : Global log data for MassiveLogger.
* `rank`      : e.g., worker ID or thread ID.
* `begin_ptr` : The return value of `MLOG_BEGIN` function.
* `decoder` : Function pointer to a decorder function that transfers recorded data into formatted string. This is called when outputting recorded data to files. See below for more details.
* `...`       : Arguments to record.

`decoder` should be defined as follows.
```c
int decoder(mlog_data_t* md, FILE* stream, int rank0, int rank1, void* buf0, void* buf1);
```

Parameters:
* `md`     : Global log data for MassiveLogger.
* `stream` : File stream to write output.
* `rank0`  : Who calls `MLOG_BEGIN`.
* `rank1`  : Who calls `MLOG_END`.
* `buf0`   : Pointer to the beginning of the recorded arguments in `MLOG_BEGIN`.
* `buf1`   : Pointer to the beginning of the recorded arguments in `MLOG_END`.

Return value:
* Bytes of the recorded args in `buf1` (`buf1_size`).

### mlog_flush
```c
void mlog_flush(mlog_data_t* md, int rank, FILE* stream);
```

Parameters:
* `md`     : Global log data for MassiveLogger.
* `rank`   : Logs in the end buffer of `rank` are flushed.
* `stream` : Logs are written to `stream`.

### mlog_flush_all
```c
void mlog_flush_all(mlog_data_t* md, FILE* stream);
```

Parameters:
* `md`     : Global log data for MassiveLogger.
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

## Illustration of Buffers

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
                                       <----------------------------------->
                                                    buf1_size
```
