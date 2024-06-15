# ---------------------------------------------------------
# copmmon.mak : include file con definizioni generali da
# includere in tutti i Makefile per Easy Picking 4.0
# ---------------------------------------------------------
# Copyright A&L srl 2003-2024
# ---------------------------------------------------------
OS_NAME != cat $(BASE_DIRECTORY)/config/OS
$(info OS_NAME       : $(OS_NAME))

ifeq (1,${DEBUG})
	CFLAGS = -g
else
	CFLAGS = -O
endif

ifeq ($(TARGET_DIR_NAME),)
	TARGET_DIR_NAME=$(TARGET_NAME)
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_GTK)","")
	HAS_GTK != cat $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_GTK
else 
	HAS_GTK = 1
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_NCURSES)","")
	HAS_NCURSES != cat $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_NCURSES
else 
	HAS_NCURSES = 1
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_POSTGRES)","")
	HAS_POSTGRES != cat $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_POSTGRES
else 
	HAS_POSTGRES = 1
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_GW)","")
	HAS_GW != cat $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_GW
else 
	HAS_GW = 0
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_ODBC)","")
	HAS_ODBC != cat $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/config/HAS_ODBC
else 
	HAS_ODBC = 0
endif

$(info TARGET_DIR_NAME       : $(TARGET_DIR_NAME))
#$(info HAS_GW       : $(HAS_GW))
#$(info HAS_GTK      : $(HAS_GTK))
#$(info HAS_NCURSES  : $(HAS_NCURSES))
#$(info HAS_POSTGRES : $(HAS_POSTGRES))
$(info HAS_ODBC     : $(HAS_ODBC))

GTK3_LIBS   = `pkg-config gtk+-3.0 --libs`  `pkg-config gmodule-export-2.0 --libs`
GTK3_CFLAGS = `pkg-config gtk+-3.0 --cflags`  `pkg-config gmodule-export-2.0 --cflags`

GLIB_LIBS   = `pkg-config glib-2.0 --libs`
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags`

# libreria di gestione Code Messaggi, Comunicazioni Seriali ...
MSQ_LIB = -L$(BASE_DIRECTORY)/common/pmx -lPmxMsq
MSQ_INC = -I$(BASE_DIRECTORY)/common/pmx

# libreria GimpWidgets
GW_LIB = -L$(BASE_DIRECTORY)/common/gw -lGW
GW_INC = -I$(BASE_DIRECTORY)/common/gw

# libreria Easy Picking
EP_LIB = -L$(BASE_DIRECTORY)/common/ep -lEP
EP_INC = -I$(BASE_DIRECTORY)/common/ep

# libreria di gestione Database PostgreSql
PSQL_LIB = `pkg-config libpq --libs`
PSQL_INC = `pkg-config libpq --cflags`

# libreria di gestione Database ODBC
ODBC_LIB = `pkg-config odbc --libs`
ODBC_INC = `pkg-config odbc --cflags`

# libreria di gestione code messaggi ZMQ
ZMQ_LIBS    = `pkg-config libzmq --libs`
ZMQ_CFLAGS = `pkg-config libzmq --cflags`

OS_TYPE=${OSTYPE}

# libreria di gestione ncurses per char terminal
ifeq ("windows","$(OS_NAME)")
	NCURSES_LIBS   = `pkg-config panelw --libs`   `pkg-config formw --libs`    `pkg-config menuw --libs`   `pkg-config ncursesw --libs`
	NCURSES_CFLAGS = `pkg-config panelw --cflags` `pkg-config formw --cflags`  `pkg-config menuw --cflags` `pkg-config ncursesw --cflags`
else
	NCURSES_LIBS   = `pkg-config panel --libs`   `pkg-config form --libs`    `pkg-config menu --libs`   `pkg-config ncurses --libs`
	NCURSES_CFLAGS = `pkg-config panel --cflags` `pkg-config form --cflags`  `pkg-config menu --cflags` `pkg-config ncurses --cflags`
endif

# libreria di gestione ncurses per char terminal
MEMCACHED_LIBS   = `pkg-config libmemcached --libs`
MEMCACHED_CFLAGS = `pkg-config libmemcached --cflags`

# Direttorio degli eseguibili
EXE_DIR=$(BASE_DIRECTORY)/exe

LFLAGS = $(EP_LIB) $(MSQ_LIB) $(ZMQ_LIBS) $(GLIB_LIBS)  -lm 
CFLAGS += $(EP_INC) $(MSQ_INC) $(ZMQ_CFLAGS) $(GLIB_CFLAGS) 

ifeq (1,${HAS_GW})
	LFLAGS += $(GW_LIB)
	CFLAGS += $(GW_INC)
endif

COMPILE_RESOURCES=0
ifeq (1,${HAS_GTK})
	LFLAGS += $(GTK3_LIBS)
	CFLAGS += $(GTK3_CFLAGS)
	COMPILE_RESOURCES=1
else
	ifeq (1,${HAS_GTK4})
		LFLAGS += $(GTK4_LIBS)
		CFLAGS += $(GTK4_CFLAGS)
		COMPILE_RESOURCES=1
	endif
endif


ifeq (1,${HAS_NCURSES})
	LFLAGS += $(NCURSES_LIBS)
	CFLAGS += $(NCURSES_CFLAGS)
endif

ifeq (1,${HAS_POSTGRES})
	LFLAGS += $(PSQL_LIB)
	CFLAGS += $(PSQL_INC)
endif

ifeq (1,${HAS_ODBC})
	LFLAGS += $(ODBC_LIB)
	CFLAGS += $(ODBC_INC)
endif

ifneq ("$(wildcard $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/VERSION)","")
	VERSION != cat VERSION
else
	VERSION =
endif

__CMP_TIME__C__=@/bin/echo -en '\
char rcsid[]="$$Id: '$(TARGET_NAME)' '`date +"%d/%m/%Y %H:%M:%S"` '$$";\n\
char __application_name__[]="'`cat ../doc/APPLICATION_NAME`'";\n\
char __module_name__[] = "'$(TARGET_NAME)'";\n\
char __module_version__[] = "'$(VERSION)'";\n\
char __version__[]="'`cat ../doc/BASE_VERSION`'";\n\
char __customer__[]="'`cat ../doc/CUSTOMER`'";\n\
char __authors__[]="'`cat ../doc/AUTHORS`'";\n\
char __configuration_file__[]="'`cat ../doc/CONFIGURATION_FILE`'";\n\
char __copyright__[]="'`cat ../doc/COPYRIGHT`'";\n'> srcs/cmp_time.c


__CMP_TIME__H__=@/bin/echo -e  '\
\#pragma once\n\
extern char rcsid[];\n\
extern char __application_name__[];\n\
extern char __module_name__[];\n\
extern char __module_version__[];\n\
extern char __version__[];\n\
extern char __customer__[];\n\
extern char __authors__[];\n\
extern char __configuration_file__[];\n\
extern char __copyright__[];\n'> includes/cmp_time.h

_OBJS_ = $(addprefix objs/,cmp_time.o)

CC=gcc
LD=gcc
GLIB_COMPILE_RESOURCES = glib-compile-resources

WARNINGS = \
	-Wall \
	-Wno-unused-variable \
	-Wno-format-overflow \
	-Wno-unused-but-set-variable \
	-Wno-misleading-indentation \
	-Wno-format-security \
	-Wno-unused-function \
	-Wno-format-zero-length \
	-Wno-unused-result
	
DEFINES = -DTRACE $(WARNINGS)
LIBS =$(EP_LIB) $(MSQ_LIB) $(PSQL_LIB) $(GLIB_LIBS) $(ZMQ_LIBS) -lm

dir_guard=@mkdir -p $(@D)

TARGET_INCLUDES_DIRECTORY = includes

INCLUDES = -I $(TARGET_INCLUDES_DIRECTORY) 

ifeq (1,${COMPILE_RESOURCES}) 
srcs/resources.c: $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/ui/$(TARGET_NAME).gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/ui --generate-dependencies $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/ui/$(TARGET_NAME).gresource.xml)
	@echo 'R - $<'
	@$(GLIB_COMPILE_RESOURCES) $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/ui/$(TARGET_NAME).gresource.xml --target=$@ --sourcedir=$(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/ui --generate-source

GLIB_COMPILE_SCHEMAS = glib-compile-schemas 

gsettings_SCHEMAS = $(BASE_DIRECTORY)/exe/org.easy-picking.$(TARGET_NAME).gschema.xml

%.gschema.valid: %.gschema.xml
	@echo 'V - $<'
	@$(GLIB_COMPILE_SCHEMAS) --strict --dry-run --schema-file=$< && mkdir -p $(@D) && touch $@

$(BASE_DIRECTORY)/exe/gschemas.compiled: $(gsettings_SCHEMAS)
	@echo 'S - $<'
	@$(GLIB_COMPILE_SCHEMAS) $(BASE_DIRECTORY)/exe
all: $(TARGET_EXECUTABLE) $(gsettings_SCHEMAS:.xml=.valid) $(BASE_DIRECTORY)/exe/gschemas.compiled
else
all: $(TARGET_EXECUTABLE)
endif



# compile time data -> objs
includes/cmp_time.h:
	@$(dir_guard)
	@$(__CMP_TIME__H__)

srcs/cmp_time.c: $(addprefix srcs/,$(TARGET_SRCS)) $(addprefix includes/,$(TARGET_INCLUDES)) includes/cmp_time.h
	@$(dir_guard)
	@$(__BUILD_INFO__)
	@$(__CMP_TIME__C__)

objs/cmp_time.o: srcs/cmp_time.c includes/cmp_time.h
	@echo 'C CMP_TIME - $<'
	@$(dir_guard)
	@$(CC) $(DEFINES) $(CFLAGS) $(INCLUDES) -c "$<" -o objs/`basename "$@"`

# Default compile rule
objs/%.o: srcs/%.c  $(addprefix includes/,$(TARGET_INCLUDES))
	@echo 'C - $<'
	@$(dir_guard)
	@$(CC) $(DEFINES) $(CFLAGS) $(INCLUDES) -c "$<" -o objs/`basename "$@"`

_SRCS_OBJS_=$(addprefix objs/,$(TARGET_SRCS:.c=.o))

TARGET_EXECUTABLE = $(BASE_DIRECTORY)/exe/$(TARGET_NAME)

$(TARGET_EXECUTABLE): $(_OBJS_) $(_SRCS_OBJS_)
	@echo 'L - $@'
	@$(dir_guard)
	@$(LD) $(_OBJS_) $(_SRCS_OBJS_) -o $(TARGET_EXECUTABLE) $(LFLAGS)

all: $(TARGET_EXECUTABLE)

clean: 
	@rm -f $(_OBJS_) $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/srcs/cmp_time.c $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/includes/cmp_time.h $(_SRCS_OBJS_) $(TARGET_EXECUTABLE) $(BASE_DIRECTORY)/$(TARGET_DIR_NAME)/srcs/resources.c

.PHONY: all clean install release doc

