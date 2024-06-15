# ---------------------------------------------------------
# target makefile
# ---------------------------------------------------------
#
TOPTARGETS := all clean install release 

TARGET_NAME = edit

BASE_DIRECTORY = ..

TARGET_SRCS = \
	main.c \
	mainvar.c \
	mainfun.c \
	mainproc.c \
	mainapp.c \
	mainwin.c \
	edit_ordine.c \
	edit_collo.c \
	edit_imballo.c \
	edit_prodotto.c \
	edit_ubicazione.c \
	edit_operatore.c \
	edit_settore.c \
	resources.c

TARGET_INCLUDES =  \
	edit_collo.h \
	edit_imballo.h \
	edit_operatore.h \
	edit_ordine.h \
	edit_prodotto.h \
	edit_settore.h \
	edit_ubicazione.h \
	mainapp.h \
	mainext.h \
	mainfun.h \
	mainstruct.h \
	mainwin.h


include $(BASE_DIRECTORY)/common/common.mak

.PHONY: $(TOPTARGETS)
