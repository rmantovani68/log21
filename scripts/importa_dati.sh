#!/bin/bash

gdialog --title "conferma copia file scarti da floppy" --yesno 'confermi la copia dei dati contenuti nel floppy ?' 300 300

case $? in

0)
	mount /mnt/floppy
	cp  /mnt/floppy/* /u/prj/mondadori/export 
	umount /mnt/floppy
	gdialog --title "copia da floppy disk terminata" --msgbox 'copia da floppy disk terminata' 350 350
	;;
1)
	:
	;;
esac

