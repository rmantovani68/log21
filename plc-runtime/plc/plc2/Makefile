
CC=gcc
CFLAGS=-g  \
	-I. -I/usr/local/cni/include  \
	-I/usr/X11R6/include \
	-DLinux
LD=gcc
LDFLAGS=-g -L/usr/local/cni/lib -L/usr/X11R6/lib

SRCS= \
	glue.c \
	ca_xpm.c \
	linux-nlist.c \
	cbkplc2.c \
	funplc.c \
	ladata.c \
	mainplc2.c
OBJS= \
	glue.o \
	ca_xpm.o \
	linux-nlist.o \
	cbkplc2.o \
	funplc.o \
	ladata.o \
	mainplc2.o

plc2: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o plc2 \
		-lxad \
		-lXm -lXp -lXt -lXpm -lXext -lX11 -lSM -lICE \
		-ldl -lplc -ldevcni

$(OBJS): equat.h pdbxad.h

