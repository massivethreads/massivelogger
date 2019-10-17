CC      := gcc
CFLAGS  := -I../../include -O3
LDFLAGS := -lpthread

os_scheduling: os_scheduling.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o os_scheduling
