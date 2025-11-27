#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS =  -O2 -Wall # DELETE -g -fsanitize=address later

OBJS = mdriver.o mm.o memlib.o pagemap.o fsecs.o fcyc.o clock.o ftimer.o

all: mdriver

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS) -lm

test: test.o mm.o memlib.o pagemap.o fcyc.o clock.o ftimer.o
	$(CC) $(CFLAGS) -o test test.o mm.o memlib.o pagemap.o fcyc.o clock.o ftimer.o -lm

test.o: test.c mm.h memlib.h
mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h pagemap.h
pagemap.o: pagemap.c pagemap.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o mdriver
