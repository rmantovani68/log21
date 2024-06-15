#!/bin/bash
gdialog --title "Cancellazione Stampe in corso" --yesno "Confermi la cancellazione delle stampe in corso?" 350 350
#echo $?
case $? in
	0) 
	/u/prj/mondadori/scripts/delete_stampe.sh
	gdialog --title "Cancellazione Stampe Terminata" --msgbox "Cancellazione delle stampe Terminata" 350 350
	;;
	1) 
		:
	;;
esac
