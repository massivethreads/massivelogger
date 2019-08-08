#define _XOPEN_SOURCE 700 // To use clock_gettime in c99
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#define ASSERT(cond, ...) \
  if (!(cond)) { \
    _print_error(__FILE__, __LINE__, __VA_ARGS__); \
    exit(1); \
  }

static inline void _print_error(const char* filename, int linenumber, const char* format, ...) {
  fprintf(stderr, "\x1b[31m"); // red

  fprintf(stderr, "[ERROR] %s:%d\n", filename, linenumber);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\x1b[39m"); // default
}
