PLCHOME=/usr/local/cni/edit_plc
PLCBIN=$(PLCHOME)/bin.Linux
PLCLIB=$(PLCHOME)/util
AS=$(PLCBIN)/as

#PLC_PROG=log21R
PLC_PROG=bistabile

all: plc-objs/user.oo

plc-objs/user.oo:	user.o
	@echo '------ creating PLC : $(PLC_PROG)'
	@$(PLCHOME)/c2a user.o -o plc-objs/user.oo
	@$(PLCHOME)/c2a user.o -o plc-objs/$(PLC_PROG).oo

#user.o:	$(PLC_PROG)_plc.o $(PLC_PROG)_msd.o scambdin_plc.o
#	@$(PLCBIN)/ld -r $(PLCHOME)/ilc0.o $(PLC_PROG)_plc.o $(PLC_PROG)_msd.o scambdin_plc.o $(PLCHOME)/ilcn.o $(PLCHOME)/libil.a -o user.o

user.o:	$(PLC_PROG)_plc.o $(PLC_PROG)_msd.o
	@$(PLCBIN)/ld -r $(PLCHOME)/ilc0.o $(PLC_PROG)_plc.o $(PLC_PROG)_msd.o $(PLCHOME)/ilcn.o $(PLCHOME)/libil.a -o user.o

$(PLC_PROG)_plc.o:	$(PLC_PROG)_plc.s
	@$(AS) $(PLC_PROG)_plc.s -o $(PLC_PROG)_plc.o

$(PLC_PROG)_msd.o:	$(PLC_PROG)_msd.s
	@$(AS) $(PLC_PROG)_msd.s -o $(PLC_PROG)_msd.o

scambdin_plc.o:	scambdin_plc.s
	@$(AS) scambdin_plc.s -o scambdin_plc.o

#$(PLC_PROG)_plc.s:	$(PLC_PROG).plc $(PLC_PROG).msd scambdin.plc.$(PLC_PROG)
	#@$(PLCHOME)/ilc -g -bound-chk 2 -header $(PLCLIB)/illib.ilc $(PLC_PROG).msd scambdin.plc.$(PLC_PROG) -code $(PLC_PROG).plc -os $(PLC_PROG)_plc.s

#$(PLC_PROG)_msd.s:	$(PLC_PROG).msd $(PLC_PROG).plc scambdin.plc.$(PLC_PROG)
	#@$(PLCHOME)/ilc -g -bound-chk 2 -header $(PLCLIB)/illib.ilc $(PLC_PROG).plc scambdin.plc.$(PLC_PROG) -code $(PLC_PROG).msd -os $(PLC_PROG)_msd.s

$(PLC_PROG)_plc.s:	$(PLC_PROG).plc $(PLC_PROG).msd
	@$(PLCHOME)/ilc -g -bound-chk 2 -header $(PLCLIB)/illib.ilc $(PLC_PROG).msd -code $(PLC_PROG).plc -os $(PLC_PROG)_plc.s

$(PLC_PROG)_msd.s:	$(PLC_PROG).msd $(PLC_PROG).plc
	@$(PLCHOME)/ilc -g -bound-chk 2 -header $(PLCLIB)/illib.ilc $(PLC_PROG).plc -code $(PLC_PROG).msd -os $(PLC_PROG)_msd.s

scambdin_plc.s:	scambdin.plc.$(PLC_PROG)
	@$(PLCHOME)/ilc -g -bound-chk 2 -code scambdin.plc.$(PLC_PROG) -os scambdin_plc.s

clean:
	@rm -f *.s *.o plc-objs/*.oo


