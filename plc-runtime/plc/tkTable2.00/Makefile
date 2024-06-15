# Generated automatically from Makefile.in by configure.
# version info
include version.in

# This sets the name that tkTable will define for itself when loaded
# If you change this, then the demos won't work, but it might be necessary
# for those with another built-in "table" command
TBL_COMMAND	= table

SRCDIR		= .

PACKAGE		= Tktable
OBJS		= tkTable.o tkTableWin.o tkTableTag.o tkTableCell.o cmd.o
MANS		= $(SRCDIR)/tkTable.n

#-------------------------------------------------
prefix		= /usr/local
exec_prefix	= /usr/local

TCL_VERSION	= 8.0
TK_VERSION	= 8.0

TCL_SRC_DIR 	= /usr/src/bs/BUILD/tcltk-8.0.5/tcl8.0.5
TK_SRC_DIR 	= /usr/src/bs/BUILD/tcltk-8.0.5/tk8.0.5

TCL_BUILD_LIB_SPEC	= -L/usr/src/bs/BUILD/tcltk-8.0.5/tcl8.0.5/unix -ltcl8.0${TCL_DBGX}
TK_BUILD_LIB_SPEC	= -L/usr/src/bs/BUILD/tcltk-8.0.5/tk8.0.5/unix -ltk8.0${TK_DBGX}

TCL_LIB_SPEC	= -L/usr/lib -ltcl8.0${TCL_DBGX}
TK_LIB_SPEC	= -L/usr/lib -ltk8.0${TK_DBGX}

TCL_LIBS	= -ldl  -lm
TK_LIBS		= -L/usr/X11R6/lib -lX11 -ldl  -lm

INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644
CC		= gcc -pipe
RANLIB		= :
SHLIB_CFLAGS	= -fPIC
SHLIB_SUFFIX	= .so
SHLIB_LD	= gcc -pipe -shared
TCL_PREFIX	= /usr
## HP cc sometimes requires -Aa for proper ansi compilation
TCL_CFLAGS	= -O  -DHAVE_GETCWD=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_TIME_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_TM_ZONE=1 -DHAVE_TM_GMTOFF=1 -DHAVE_TIMEZONE_VAR=1 -DHAVE_ST_BLKSIZE=1 -DSTDC_HEADERS=1 -DNEED_MATHERR=1 -DRETSIGTYPE=void -DHAVE_SIGNED_CHAR=1 -DHAVE_SYS_IOCTL_H=1  
TCL_LD_FLAGS	= -rdynamic
TK_LD_SEARCH_FLAGS = 
TK_XINCLUDES	= -I/usr/X11R6/include

#TCL_CFLAGS	+= -DDEBUG -g -Wall -Wno-implicit

LIB_RUNTIME_DIR = /usr/lib

INCLUDES	= -I${prefix}/include $(TK_XINCLUDES)\
		  -I$(TCL_SRC_DIR)/generic\
		  -I$(TK_SRC_DIR)/generic

DLL		= $(PACKAGE)$(SHLIB_SUFFIX)
STATIC_LIB	= $(PACKAGE).a
PACKAGEDIR	= $(PACKAGE)$(TBL_VERSION)
DLLDIR		= ${exec_prefix}/lib/$(PACKAGEDIR)
RTDIR		= ${prefix}/lib/$(PACKAGEDIR)

CFLAGS		= -I. -I$(SRCDIR) $(TCL_CFLAGS) $(SHLIB_CFLAGS) $(INCLUDES)\
		  -DTBL_VERSION=\"$(TBL_VERSION)\"\
		  -DTBL_COMMAND=\"$(TBL_COMMAND)\"\
		  -DTCL_RUNTIME=\"$(RTDIR)/tkTable.tcl\"

MAN_INSTALL_DIR = ${prefix}/man/mann

all: $(DLL) pkgIndex.tcl

$(DLL): $(OBJS)
	$(SHLIB_LD) -o $@ $(OBJS)

static $(STATIC_LIB): $(OBJS) pkgIndex.tcl
	rm -f $(STATIC_LIB)
	ar cr $(STATIC_LIB) $(OBJS)
	$(RANLIB) $(STATIC_LIB)

cmd.o:		cmd.h cmd.c
tkTable.o:	$(SRCDIR)/tkTable.h $(SRCDIR)/tkTable.c tkTabletcl.h
tkTableCell.o:	$(SRCDIR)/tkTable.h $(SRCDIR)/tkTableCell.c
tkTableTag.o:	$(SRCDIR)/tkTable.h $(SRCDIR)/tkTableTag.c
tkTableWin.o:	$(SRCDIR)/tkTable.h $(SRCDIR)/tkTableWin.c
tkAppInit.o:	$(SRCDIR)/tkAppInit.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRCDIR)/$*.c

pkgIndex.tcl:
	(\
	echo 'if {[catch {package require Tcl $(TCL_VERSION)}]} return';\
	echo 'package ifneeded $(PACKAGE) $(TBL_VERSION)\
	    "package require Tk $(TK_VERSION);\
	     [list load [file join $$dir $(DLL).$(TBL_VERSION)] $(PACKAGE)]"'\
	) > pkgIndex.tcl

install: all
	if test ! -d "$(DLLDIR)"; then mkdir "$(DLLDIR)"; fi
	if test ! -d "$(RTDIR)"; then mkdir "$(RTDIR)"; fi
	$(INSTALL_PROGRAM) $(DLL) "$(DLLDIR)/$(DLL).$(TBL_VERSION)"
	$(INSTALL_DATA) pkgIndex.tcl "$(DLLDIR)/pkgIndex.tcl"
	$(INSTALL_DATA) $(SRCDIR)/tkTable.tcl "$(RTDIR)/tkTable.tcl"
	$(INSTALL_DATA) $(MANS) $(MAN_INSTALL_DIR)/

test: all
	true

clean: 
	rm -f *.o $(DLL) $(STATIC_LIB) core pkgIndex.tcl tkTabletcl.h tablewish

distclean: clean
	rm -f Makefile config.cache config.log config.status

tkTabletcl.h: $(SRCDIR)/tkTable.tcl
	sed -e '/^$\#/d' -e '/^$$/d' -e 's/\"/\\"/g' -e 's/^/"/' -e 's/$$/\\n"/' <$(SRCDIR)/tkTable.tcl >tkTabletcl.h || rm tkTabletcl.h

tablewish: tkAppInit.o $(OBJS)
	$(CC) $(TK_LD_SEARCH_FLAGS) -o $@ tkAppInit.o $(OBJS) \
	  $(LIBS) $(TK_BUILD_LIB_SPEC) $(TK_LIB_SPEC) $(TCL_BUILD_LIB_SPEC) $(TCL_LIB_SPEC) $(TK_LIBS) $(TCL_LIBS)
