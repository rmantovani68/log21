#!/bin/bash
gdialog --title "Arresto server Database" --yesno "Confermi lo spegnimento del server DataBase" 350 350
#echo $?
case $? in
	0) 
#		rsh database /sbin/poweroff;
		rsh database /usr/local/bin/spegni.sh;
	;;
	1) 
		:
	;;
esac
