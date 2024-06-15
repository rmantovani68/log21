#!/bin/bash

gdialog --title "formatta floppy" --yesno 'confermi la formattazione del floppy ?' 300 300

case $? in

0)
	mformat a:
	gdialog --title "formattazione floppy disk terminata" --msgbox 'formattazione floppy disk terminata' 350 350
	;;
1)
	:
	;;
esac

