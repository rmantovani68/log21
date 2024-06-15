#!/bin/bash

PS_AX=$(ps ax)
#echo -e $PS_AX
MAIN_PROCESS=-1



function start_main() {

	ps ax  | while read a b c d e f g h i l; do case "$e" in */main* ) echo trovato $a $e $f $g $h $i $l; kill -15 $a;;  esac; done
	ps ax  | while read a b c d e f g h i l; do case "$e" in */volum* ) echo trovato $a $e $f $g $h $i $l; kill -15 $a;;  esac; done
	ps ax  | while read a b c d e f g h i l; do case "$e" in */receive* ) echo trovato $a $e $f $g $h $i $l; kill -15 $a;;  esac; done
	ps ax  | while read a b c d e f g h i l; do case "$e$g" in */printlabel4* ) echo trovato $a $e $f $g $h $i $l; kill -15 $a;;  esac; done
	ipcs -q | while read a b c; do case "$a" in 0x00000500) echo ipcrm msg $b; ipcrm msg $b ; ;;  esac; done
	ipcs -q | while read a b c; do case "$a" in 0x00000503) echo ipcrm msg $b; ipcrm msg $b ; ;;  esac; done
	ipcs -q | while read a b c; do case "$a" in 0x00000504) echo ipcrm msg $b; ipcrm msg $b ; ;;  esac; done
	ipcs -q | while read a b c; do case "$a" in 0x00000508) echo ipcrm msg $b; ipcrm msg $b ; ;;  esac; done

	#cd /u/prj/mondadori-ufficio/exe
	cd /u/prj/mondadori-rcs-ufficio/exe
	./main  &
}


function processo_main_esistente() {
	if [[ $5 == *./main* ]]
	then
		echo trovato $1 $5
		MAIN_PROCESS=$1
	fi
}


old_IFS=$IFS      # save the field separator           
IFS=$'\n'     # new field separator, the end of line           
for line in $PS_AX      
do
	IFS=$old_IFS     # restore default field separator
	processo_main_esistente $line
done          
IFS=$old_IFS     # restore default field separator

echo $MAIN_PROCESS

if [ "$MAIN_PROCESS" -gt "1" ]
then
	zenity --title="Main process Presente" --question --text "Esiste gia' un'istanza attiva di main.\nTerminarla e proseguire con una nuova istanza?" width=300 height=300 
	case $? in
	0)
		echo ammazzo main e rientro
		kill -15 $MAIN_PROCESS
		sleep 5
		start_main
		exit 0
		;;
	1)
		echo Non c'e' main e non entro
		exit 0
		;;
	esac
else
	echo Non c'e' main e entro
	start_main
	exit 0
fi


exit 0
