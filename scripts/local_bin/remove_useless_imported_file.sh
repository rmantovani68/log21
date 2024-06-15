#!/bin/bash
#
# remove_useless_imported_file.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# remove_useless_imported_file.sh --filename papni002.txt --importdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-import

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

File_Name=""
Import_Dir=""
Import_File_Name=""

Current_Date=`date`

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "--filename <file>"
	echo "--importdir <dir>"
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
		--*) ;;
		*)  ;;
	esac
	shift
done

if [[ -z $File_Name ]] || [[ -z $Import_Dir ]]
then
	print_help
	exit 1
fi

cd $Import_Dir
if [ -f `basename $File_Name .txt`* ]
then
	Import_File_Name=$(basename $(ls -rt $Import_Dir/$(basename $File_Name .txt)* |head -1))
	echo "the import file exists: $Import_File_Name. Deleting it..."
	chmod a+rwx $Import_Dir/$Import_File_Name
	rm $Import_Dir/$Import_File_Name 
	Rm_Exit_Status=$?
else
	echo "the import file doesn't exists"
fi


case $Rm_Exit_Status in
0)
	Message="Cancellato file $Import_File_Name da $Import_Dir"
	echo "insert into ftp_receive_log (message,code) values ('$Message',$Rm_Exit_Status)"|psql -h database mondadori 
	;;
*)
	Message="Errore generico"
	;;
esac

exit $Rm_Exit_Status 

