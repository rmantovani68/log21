#!/bin/bash


gdialog --title "Conferma Cancellazione Scarti" --yesno 'Confermi cancellazione \nTabella di storico Scarti?' 300 300 

case $? in

0)
	/u/prj/mondadori/scripts/delete_scarti.sh
	gdialog --title "Cancellazione Scarti Terminata" --msgbox "Cancellazione della tabella scarti Terminata" 350 350
	;;
1)
		:
	;;
esac
