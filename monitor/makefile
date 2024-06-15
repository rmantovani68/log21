# ---------------------------------------------------------
# target makefile
# ---------------------------------------------------------
#
TOPTARGETS := all clean install release 

TARGET_NAME = monitor

BASE_DIRECTORY = ..

TARGET_SRCS = \
	main.c \
	mainvar.c \
	mainfun.c \
	mainproc.c \
	mainapp.c \
	mainwin.c \
	resources.c 

TARGET_INCLUDES = \
	mainapp.h \
	mainext.h \
	mainfun.h \
	mainstruct.h \
	mainwin.h


include $(BASE_DIRECTORY)/common/common.mak

.PHONY: $(TOPTARGETS)
