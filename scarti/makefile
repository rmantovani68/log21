# ---------------------------------------------------------
# Makefile for Easy Picking - Main Process
# ---------------------------------------------------------
# Copyright A&L srl 2003-2021

# --------------------------
# Project Name (executable)
# --------------------------
PRJ_NAME = scarti

# --------------------------
# Program debug flags
# --------------------------

# debug version
DEFDEBUG = -g -DDEBUG -DTRACE

# release version
#DEFDEBUG = -O 

# --------------------------
# user defines
# --------------------------

# User Definitions
USR_DEF =  -DTRACE 

# include file contenente le definizioni generiche
include ../common/picking.mak

# sorgenti componenti il progetto
SRCS = \
	main.c \
	mainvar.c \
	mainfun.c \
	mainproc.c \
	mainapp.c \
	mainwin.c \
	resources.c

# libraries
LIBS = $(GTK3_LIBS) $(EP_LIB) $(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GIMP_LIBS) $(ZMQ_LIBS)

# includes 
INCLUDES = -I.  $(GTK3_CFLAGS) $(EP_INC) $(MSQ_INC) $(EP_INC) $(PSQL_INC)


BUILT_SOURCES = resources.c $(EXE_DIR)/gschemas.compiled

gsettings_SCHEMAS = $(EXE_DIR)/org.easy-picking.$(PRJ_NAME).gschema.xml

# main target
all: $(gsettings_SCHEMAS:.xml=.valid) $(BUILT_SOURCES) $(EXE_DIR)/$(PRJ_NAME)





# dependencies



cmp_time.h:  $(SRCS:.c=.o)
	$(__CMP_TIME__H__)



main.o: main.c mainapp.h

mainapp.o: mainapp.c mainapp.h mainwin.h

mainwin.o: mainwin.c mainapp.h mainwin.h mainstruct.h mainext.h




mainvar.o: mainvar.c  mainstruct.h mainext.h

mainfun.o:  mainfun.c mainfun.h  mainstruct.h mainext.h

mainproc.o:  mainproc.c mainfun.h  mainstruct.h mainext.h

resources.o: resources.c

srcdir=.
GLIB_COMPILE_RESOURCES = glib-compile-resources

resources.c: $(PRJ_NAME).gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/$(PRJ_NAME).gresource.xml)
	@echo 'R - $<'
	@$(GLIB_COMPILE_RESOURCES) $(srcdir)/$(PRJ_NAME).gresource.xml --target=$@ --sourcedir=$(srcdir) --generate-source

GLIB_COMPILE_SCHEMAS = glib-compile-schemas 

%.gschema.valid: %.gschema.xml
	@echo 'V - $<'
	@$(GLIB_COMPILE_SCHEMAS) --strict --dry-run --schema-file=$< && mkdir -p $(@D) && touch $@

$(EXE_DIR)/gschemas.compiled: $(gsettings_SCHEMAS)
	@echo 'S - $<'
	@$(GLIB_COMPILE_SCHEMAS) $(EXE_DIR)


# build rules
$(EXE_DIR)/$(PRJ_NAME): $(SRCS:.c=.o)
	$(__CMP_TIME__)
	@echo 'Compiling cmp_time.c'
	@$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c cmp_time.c -o cmp_time.o
	@echo 'Linking ... $@'
	@$(CC) $(SRCS:.c=.o) cmp_time.o $(LIBS) -o $(EXE_DIR)/$(PRJ_NAME)
	@rm -rf $(EXE_DIR)/org.easy-picking.$(PRJ_NAME).gschema.valid
	@echo 'Terminated'

clean:
	@echo '------ Clean ...'
	@rm -f $(SRCS:.c=.o) $(BUILT_SOURCES) $(EXE_DIR)/$(PRJ_NAME) $(gsettings_SCHEMAS:.xml=.valid)
	@echo '------ Terminated'


