# ---------------------------------------
# Makefile per Easypicking
# ---------------------------------------
# 
COMMON_DEPS = common/ep/libEP.a

TOPTARGETS := all clean

SUBDIRS = \
	common \
	shm_db_sync \
	main \
	barcode \
	bilancia \
	ios \
	peso \
	printlabel \
	settori \
	volum \
	receive-rcs \
	balance \
	monitor \
	bancali \
	check_bancali \
	auto_tasks \
	dbf2sql \
	display \
	test \
	setup \
	carichi \
	edit \
	template

#	socket_server \
#	fine_linea_CEVA \
#	readshm \
#	writeshm \

#	ev2002 \

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

release: VERSIONE

VERSIONE: $(SOURCES) $(HEADERS) makefile 
	@if [ -r doc/BASE_VERSION ]; then \
		versione=`awk '{ split($$0,a,"."); print a[1] "." a[2] "." a[3] "." (a[4]+1); }' doc/BASE_VERSION` ;\
	else \
		versione=1.0.0.0; \
	fi; \
	echo 'Numero di versione : '"$$versione"; \
	echo "$$versione"  > doc/BASE_VERSION


.PHONY: $(TOPTARGETS) $(SUBDIRS)
