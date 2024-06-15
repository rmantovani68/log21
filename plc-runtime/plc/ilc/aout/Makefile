

OS=$(shell uname)

PROGS=c2a astrip
BINS=$(patsubst %,../bin/$(OS)/%,$(PROGS))

$(BINS): $(PROGS)
	cp $(PROGS) ../bin/$(OS)/

include Makefile.$(OS)

clean:
	rm -f $(PROGS)

.PHONY: clean

