CC=gcc
CFLAGS=-g -I/usr/local/cni/include \
          -L/usr/local/cni/lib

LIBS=-lreadline -ldevcni -lplc

tester: tester.c
	$(CC) $(CFLAGS) tester.c $(LIBS) -o tester
