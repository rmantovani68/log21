
SHELL=/bin/bash
CFLAGS=-g -I.. -I/usr/local/cni/include -L.. -L/usr/local/cni/lib
LIBS=-lplc -ldevcni
CC=gcc

esempio:	esempio.c ../libplc.a
	$(CC) $(CFLAGS) esempio.c -o esempio $(LIBS)
