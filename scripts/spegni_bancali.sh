#!/bin/bash
gdialog --title "Arresto Stazione Bancali" --yesno "Confermi lo spegnimento della stazione Bancali ?" 350 350
#echo $?
case $? in
	0) 
		rsh bancali /usr/local/bin/spegni.sh;
	;;
	1) 
		:
	;;
esac
