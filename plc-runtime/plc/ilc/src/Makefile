
OS=$(shell uname)

../bin/$(OS)/ilc: 	ilc
	cp ilc ../bin/$(OS)/ilc

include Makefile.$(OS)

clean:
	rm -f *.o ilc

.PHONY: clean

