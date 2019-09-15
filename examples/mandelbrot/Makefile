CC      := gcc
CFLAGS  := -I../../include -O3 -fopenmp
LDFLAGS := -lm

mandelbrot: mandelbrot.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o mandelbrot
