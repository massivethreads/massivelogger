# OS Scheduling

This example visualizes how threads are scheduled by the OS.
By using low-level APIs of MassiveLogger, we can easily record OS timeslices.

## How to Run

```sh
make
./os_scheduling
```
Then `mlog.txt` is generated.
It is an output file of MassiveLogger.

You can change the number of threads and duration of them. To see a help:
```sh
./os_scheduling -h
```

## Visualize Timeline by MassiveLogger

```sh
../../run_viewer.bash mlog.txt
```
