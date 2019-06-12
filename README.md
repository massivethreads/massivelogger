# MassiveLogger

Lightweight Logging Library for Multi-Threading.

## API

### mlog_begin
```c
void* mlog_begin(int rank, int (*decoder)(FILE*, uint64_t, uint64_t, int, int, void*, void*), ...);
```

Parameters:
* `rank`    : e.g., worker ID or thread ID.
* `decoder` : Function pointer to a decorder function that transfers recorded data into formatted string. This is called when outputting recorded data to files. See below for more details.
* `...`     : Arguments to record.

Return value:
* A pointer to the recorded data (`begin_ptr`). This should be passed to `mlog_end` function.

`decoder` should be defined as follows.
```c
int decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1);
```

Parameters:
* `stream` : File stream to write output.
* `rank0`  : Who calls `mlog_begin`.
* `rank1`  : Who calls `mlog_end`.
* `buf0`   : Pointer to the beginning of the recorded arguments in `mlog_begin`.
* `buf1`   : Pointer to the beginning of the recorded arguments in `mlog_end`.

Return value:
* Bytes of the recorded args in `buf1` (`buf1_size`).

### mlog_end

```c
void mlog_end(int rank, void* begin_ptr, ...);
```

Parameters:
* `rank`      : e.g., worker ID or thread ID.
* `begin_ptr` : The return value of `mlog_begin` function.
* `...`       : Arguments to record.

## Illustration of Buffers

```
                                            buf0
                                             |
                                             v
          -----------------------------------------------------------------------------------
rank0                      |           |           |
                 ...       |   arg1    |   arg2    |                   ...
begin_buf                  |           |           |
          -----------------------------------------------------------------------------------
                                 ^
                                 |                      buf1
                                 |                       |
                                 |                       v
          -----------------------------------------------------------------------------------
rank1          |           |           |           |           |           |           |
           ... |  decoder  | begin_ptr |   arg1    |   arg2    |    ...    |  decoder  | ...
end_buf        |           |           |           |           |           |           |
          -----------------------------------------------------------------------------------
                                       <----------------------------------->
                                                    buf1_size
```
