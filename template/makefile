# ---------------------------------------------------------
# target makefile
# ---------------------------------------------------------
#
TOPTARGETS := all clean install release 

TARGET_NAME = template

BASE_DIRECTORY = ..

TARGET_SRCS = \
	main.c \
	mainapp.c \
	mainwin.c \
	mainvar.c \
	mainfun.c \
	mainproc.c \
	template_source.c \
	resources.c

TARGET_INCLUDES = mainwin.h mainfun.h  mainstruct.h mainext.h template_source.h

include $(BASE_DIRECTORY)/common/common.mak

.PHONY: $(TOPTARGETS)
