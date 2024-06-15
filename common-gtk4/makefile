# ---------------------------------------
# Makefile per Easypicking
# ---------------------------------------
# 

TOPTARGETS := all clean

SUBDIRS = \
	pmx \
	ep \
	gw

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

