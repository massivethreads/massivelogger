#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <complex.h>
#include <omp.h>

#include "mlog/mlog.h"

mlog_data_t g_md;

int mandelbrot(double complex c, int depth) {
  int count = 0;
  double complex z = 0;
  for (int i = 0; i < depth; i++) {
    if (cabs(z) >= 2.0) {
      break;
    }
    z = z * z + c;
    count++;
  }
  return count;
}

void mandelbrot_parallel(int* p, int nx, int ny, int depth, double scale) {
#pragma omp parallel for
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      int rank = omp_get_thread_num();
      void *begin_ptr = mlog_begin_tl(&g_md, rank);

      double sx = (double)(x - nx / 2) / nx * scale;
      double sy = (double)(y - ny / 2) / ny * scale;
      p[y * nx + x] = mandelbrot(sx + sy * I, depth);

      if (p[y * nx + x] == depth) {
        mlog_end_tl(&g_md, rank, begin_ptr, "converged");
      } else {
        mlog_end_tl(&g_md, rank, begin_ptr, "diverged");
      }
    }
  }
}

void output_mandelbrot(int* p, int nx, int ny) {
  FILE *fp = fopen("mandelbrot.txt", "w");
  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      fprintf(fp, "%d %d %d\n", x, y, p[y * nx + x]);
    }
  }
  fclose(fp);
}

void output_mlog() {
  FILE *fp = fopen("mlog.txt", "w");
  mlog_flush_all(&g_md, fp);
  fclose(fp);
}

int main(int argc, char* argv[]) {
  int nx = 2000;
  int ny = 2000;
  int depth = 100;
  double scale = 2.0;

  int n_threads;
#pragma omp parallel
  {
    #pragma omp single
    n_threads = omp_get_num_threads();
  }
  mlog_init(&g_md, n_threads, (2 << 20));

  int opt;
  while ((opt = getopt(argc, argv, "x:y:d:s:h")) != EOF) {
    switch (opt) {
      case 'x':
        nx = atoi(optarg);
        break;
      case 'y':
        ny = atoi(optarg);
        break;
      case 'd':
        depth = atoi(optarg);
        break;
      case 's':
        scale = atof(optarg);
        break;
      case 'h':
      default:
        printf("Usage: ./mandelbrot -x <nx> -y <ny> -d <depth> -s <scale>\n");
        exit(1);
    }
  }

  int* p = (int *)malloc(sizeof(int) * nx * ny);

  mandelbrot_parallel(p, nx, ny, depth, scale);

  output_mandelbrot(p, nx, ny);
  output_mlog();

  return 0;
}
