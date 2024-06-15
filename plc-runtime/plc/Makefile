
LBIN=/usr/local/bin
LUSR=/usr/local
QPLC=qplc-socketcan

all:
	@echo 'Eh, ti piacerebbe, vero? Invece devi fare tutto a mano.'

install:
	cp libftp/libpcftp.a cni/lib
	cp libftp/libpcftp.h cni/include
	cp libplc/libplc.a cni/lib
	cp libplc/plclink.h cni/include

	#mkdir $(LUSR)/cni
	#mkdir $(LUSR)/cni/include
	#mkdir $(LUSR)/cni/bin

	cp cni/include/* $(LUSR)/cni/include
	cp cni/lib/* $(LUSR)/cni/lib

	cp $(QPLC)/src/qplc $(LBIN)/qplc
	chown root $(LBIN)/qplc; chmod 755 $(LBIN)/qplc
	chmod u+s $(LBIN)/qplc
	cp wishDeb/wishDeb $(LBIN)/wishDeb
	chown root $(LBIN)/wishDeb; chmod 755 $(LBIN)/wishDeb
	tar cf - cni | ( cd $(LUSR) && tar xvf - )
	tar cf - edit_plc | ( cd $(LUSR)/cni && tar xvf - )
	find $(LUSR)/cni -exec chown root {} \;
	cp edit_plc/edit_plc.sh $(LBIN)/edit_plc
	chown root $(LBIN)/edit_plc; chmod 755 $(LBIN)/edit_plc

.PHONY:	all install

