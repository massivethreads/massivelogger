# Mandelbrot

Computation of Mandelbrot set.

This is an example where static domain decomposition causes load imbalances.
Computation of Mandelbrot set is parallelized by static parallel for loop of OpenMP.
The load imbalances are well visualized by MassiveLogger.

This example was picked from:

Michael McCool, James Reinders, and Arch Robison. 2012. Structured Parallel Programming: Patterns for Efficient Computation (1st ed.). Morgan Kaufmann Publishers Inc., San Francisco, CA, USA.

## How to Run

```sh
make
./mandelbrot
```
Then `mlog.txt` and `mandelbrot.txt` are generated.
`mlog.txt` is an output file of MassiveLogger, and `mandelbrot.txt` contains the result of the computation of Mandelbrot set.

You can change some parameters for Mandelbrot set. To see a help:
```sh
./mandelbrot -h
```

## Visualize Timeline by MassiveLogger

```sh
../../run_viewer.bash mlog.txt
```

## Visualize Mandelbrot Set

```sh
gnuplot plot.gpi
```
