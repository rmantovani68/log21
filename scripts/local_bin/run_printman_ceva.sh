#!/bin/bash

#
# rm : problemi con verifica esistenza processo, sembra che non funzioni ...
#


PS_AX=$(ps ax)
#echo -e $PS_AX
MAIN_PROCESS=-1
EXE_PATH=/u/prj/mondadori-rcs-ufficio/exe


function start_main() {
	cd $EXE_PATH
	./printman  &
}


function processo_main_esistente() {
	if [[ $5 == *./printman* ]]
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

if [ $MAIN_PROCESS -gt 1 ]
then
	echo printman presente
else
	echo printman non presente
	start_main
	exit 0
fi


exit 0
