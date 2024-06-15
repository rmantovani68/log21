#!/bin/bash

gdialog --title "conferma consuntivo scarti" --yesno 'confermi consuntivazione\ndati di storico scarti ?' 300 300

case $? in

0)
	/u/prj/mondadori/scripts/totale_scarti.sh
	gdialog --title "consuntivazione scarti terminata" --msgbox 'consuntivazione tabella di storico scarti terminata\npremere un tasto per la copia su floppy disk' 350 350
	mount /mnt/floppy
	cp  /u/prj/mondadori/export/scarti-group.txt /mnt/floppy
	umount /mnt/floppy
	gdialog --title "copia su floppy disk terminata" --msgbox 'copia su floppy disk terminata' 350 350
	;;
1)
	:
	;;
esac

