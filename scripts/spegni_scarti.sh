#!/bin/bash
gdialog --title "Arresto Stazione Scarti" --yesno "Confermi lo spegnimento della stazione Scarti ?" 350 350
#echo $?
case $? in
	0) 
		rsh scarti /usr/local/bin/spegni.sh;
	;;
	1) 
		:
	;;
esac
