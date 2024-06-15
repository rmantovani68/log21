#!/bin/bash
#
# move_imported_file_to_receive.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# move_imported_file_to_receive.sh --importdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-import 
#                                  --targetdir /u/prj/mondadori-ufficio-luca/export --filename cacni001.txt

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

File_Name=""
Export_Dir=""
Backup_Dir=""
Target_Dir=""
Export_File_Name=""
Mv_Exit_Status=1

Current_Date=`date`

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "--filename <file>"
	echo "--exportdir <dir>"
	echo "--targetdir <dir>"
	echo
}



if [ $# = $NOARGS ]
then
	print_help
	exit $E_BADARGS
fi

while [ $# != 0 ]
do
	case "$1" in
		--filename) File_Name=$2 ; shift ;;
		--exportdir) Export_Dir=$2 ; shift ;;
		--targetdir) Target_Dir=$2 ; shift ;;
		--*) ;;
		*)  ;;
	esac
	shift
done

if [[ -z $File_Name ]] || [[ -z $Export_Dir ]] || [[ -z $Target_Dir ]]
then
	print_help
	exit 1
fi

cd $Export_Dir
#Export_File_Name=$(ls -rt $File_Name.sped)
Export_File_Name=$(ls -rt $File_Name)
if [[ -z $Export_File_Name ]]
then
		echo "the export file doesn't exist"
		exit 1
else
	if [ -f $Export_File_Name ]
	then
		echo "the export file exists: $Export_File_Name"
		chmod a+rwx $Export_Dir/$Export_File_Name
		Now_String=$(date +"%Y%m%d%H%M%S")
		Target_File_Name=$(basename $File_Name .txt)_$Now_String.txt
		echo "the target file: $Target_File_Name"
		mv $Export_Dir/$Export_File_Name  $Target_Dir/$Target_File_Name
		Mv_Exit_Status=$?
	else
		echo "the export file doesn't exist"
		exit 1
	fi
fi


case $Mv_Exit_Status in
0)
	rm trasmissione.mon
	Message="Creato file $Target_File_Name da $Export_File_Name"
	echo "insert into ftp_send_log (message,code) values ('$Message',$Mv_Exit_Status)"|psql -h database mondadori 
	;;
*)
	Message="Errore generico"
	;;
esac

exit $Mv_Exit_Status 

