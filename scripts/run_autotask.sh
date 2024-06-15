#!/bin/bash
PS_AX=$(ps ax)
#echo -e $PS_AX
MAIN_PROCESS=-1


function start_main() {
	#cd /u/prj/mondadori-ufficio/exe
	cd /u/prj/mondadori/exe
	./auto_tasks -p 16  &
}


function processo_main_esistente() {
	if [[ $5 == *./auto_tasks* ]]
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
	echo auto_task gia presente
else
	echo Non c e auto_task e entro
	start_main
	exit 0
fi


exit 0
