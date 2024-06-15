# Makefile per libreria GimpWidgets

GTK_LIBS   = `pkg-config gtk+-3.0 --libs`  `pkg-config gmodule-export-2.0 --libs`
GTK_CFLAGS = `pkg-config gtk+-3.0 --cflags`  `pkg-config gmodule-export-2.0 --cflags`

GLIB_LIBS   = `pkg-config glib-2.0 --libs` 
GLIB_CFLAGS = `pkg-config glib-2.0 --cflags` 

# Flags di compilazione (debug)
WARNINGS = -Wall -Wno-unused-variable -Wno-format-overflow -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-format-security -Wno-unused-function
CFLAGS = -DGIMP_WIDGETS_COMPILATION -DGIMP_BASE_COMPILATION $(WARNINGS) -g -I.

CC = gcc

ARFLAGS = rvU

# modifico il metodo di default per la creazione della libreria
# (elimino la cancellazione del codice oggetto)
.c.a:
	@$(CC) $(GTK_CFLAGS) $(GLIB_CFLAGS) $(CFLAGS) -c -o $% $<
	@$(AR) $(ARFLAGS) $@ $%

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M -MD -MP  $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Parametri generali
GWLIB = libGW.a

GWLIBOBJS =	$(GWLIB)(gimpbase-private.o)\
					$(GWLIB)(gimpruler.o)\
					$(GWLIB)(gimpunit.o)

# Regole di compilazione della libreria per code messaggi
all:	$(GWLIB)

clean:
	touch *.[ch]
	rm -rf $(GWLIB)

library:	$(GWLIB)

$(GWLIB):	$(GWLIBOBJS)

SOURCES = gimpbase-private.c gimpruler.c gimpunit.c
