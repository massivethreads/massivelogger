# MassiveLogger

Lightweight Logging Library for Multi-Threading.

## API

### MLOG_BEGIN
```c
void* MLOG_BEGIN(int rank, int (*decoder)(FILE*, int, int, void*, void*), ...);
```

Parameters:
* `rank`    : e.g., worker ID or thread ID.
* `decoder` : Function pointer to a decorder function that transfers recorded data into formatted string. This is called when outputting recorded data to files. See below for more details.
* `...`     : Arguments to record.

Return value:
* A pointer to the recorded data (`begin_ptr`). This should be passed to `MLOG_END` function.

`decoder` should be defined as follows.
```c
int decoder(FILE* stream, int rank0, int rank1, void* buf0, void* buf1);
```

Parameters:
* `stream` : File stream to write output.
* `rank0`  : Who calls `MLOG_BEGIN`.
* `rank1`  : Who calls `MLOG_END`.
* `buf0`   : Pointer to the beginning of the recorded arguments in `MLOG_BEGIN`.
* `buf1`   : Pointer to the beginning of the recorded arguments in `MLOG_END`.

Return value:
* Bytes of the recorded args in `buf1` (`buf1_size`).

### MLOG_END

```c
void MLOG_END(int rank, void* begin_ptr, ...);
```

Parameters:
* `rank`      : e.g., worker ID or thread ID.
* `begin_ptr` : The return value of `MLOG_BEGIN` function.
* `...`       : Arguments to record.

## Illustration of Buffers

```
                                buf0
                                 |
                                 v
          -----------------------------------------------------------------------
rank0          |           |           |           |
           ... |  decoder  |   arg1    |   arg2    |            ...
begin_buf      |           |           |           |
          -----------------------------------------------------------------------
                     ^
                     |          buf1
                     |           |
                     |           v
          -----------------------------------------------------------------------
rank1          |           |           |           |           |           |
           ... | begin_ptr |   arg1    |   arg2    |    ...    | begin_ptr | ...
end_buf        |           |           |           |           |           |
          -----------------------------------------------------------------------
                           <----------------------------------->
                                        buf1_size
```
