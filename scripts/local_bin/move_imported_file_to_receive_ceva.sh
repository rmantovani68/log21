#!/bin/bash
#
# move_imported_file_to_receive_ceva.sh
# Versione: 1.0
# Autore: Luca Guerrini,Roberto Mantovani
#
# Esempio d'uso
# move_imported_file_to_receive_ceva.sh --importdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-import 
#                                  --targetdir /u/prj/mondadori-ufficio-luca/export --filename cacni001.txt

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

File_Name=""
Import_Dir=""
Backup_Dir=""
Target_Dir=""
Import_File_Name=""

Current_Date=`date`
TIME_STAMP=`date +%Y%m%d%H%M%S` 

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "--filename <file>"
	echo "--importdir <dir>"
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
		--importdir) Import_Dir=$2 ; shift ;;
		--targetdir) Target_Dir=$2 ; shift ;;
		--backupdir) Backup_Dir=$2 ; shift ;;
		--*) ;;
		*)  ;;
	esac
	shift
done



#
# controllo presenza parametri
#
if [[ -z $File_Name ]] || [[ -z $Import_Dir ]] || [[ -z $Target_Dir ]]
then
	print_help
	exit 1
fi

cd $Import_Dir

if [ -f "`basename $(ls -rt $Import_Dir/$(basename $File_Name .txt)* |head -1)`" ]
then
	Import_File_Name=$(basename $(ls -rt $Import_Dir/$(basename $File_Name .txt)* |head -1))
	echo "the import file exists: $Import_File_Name"
	cd $Target_Dir

	chmod a+rwx $Import_Dir/$Import_File_Name
	cp $Import_Dir/$Import_File_Name  $Backup_Dir/$Import_File_Name
	mv -T -f $Import_Dir/$Import_File_Name  $Target_Dir/$Import_File_Name
	Mv_Exit_Status=$?

else
	echo "the import file doesn't exists"
fi


case $Mv_Exit_Status in
0)
	Message="Creato file $Import_File_Name da $Import_File_Name"
	echo "insert into ftp_receive_log (message,code) values ('$Message',$Mv_Exit_Status)"|psql -h database mondadori 
	;;
*)
	Message="Errore generico"
	;;
esac

exit $Mv_Exit_Status 

