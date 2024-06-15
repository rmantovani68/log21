#!/bin/bash
#
# move_imported_file_to_receive.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# move_imported_file_to_receive.sh --importdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-import 
#                                  --targetdir /u/prj/mondadori-ufficio-luca/export --filename cacni001.txt
# 02-02-2016 rm+gg : controllo se presente record di quadratura

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

File_Name=""
Import_Dir=""
Backup_Dir=""
Target_Dir=""
Import_File_Name=""
Check_Quadratura=0

Current_Date=`date`


function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "--filename <file>"
	echo "--importdir <dir>"
	echo "--targetdir <dir>"
	echo "--checkquad "
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
		--checkquad) Check_Quadratura=1 ;;
		--*) ;;
		*)  ;;
	esac
	shift
done



if [[ -z $File_Name ]] || [[ -z $Import_Dir ]] || [[ -z $Target_Dir ]]
then
	print_help
	exit 1
fi

cd $Import_Dir
#if [ -f `basename $File_Name .txt`* ]
if [ -f "`basename $(ls -rt $Import_Dir/$(basename $File_Name .txt)* |head -1)`" ]
then
	Import_File_Name=$(basename $(ls -rt $Import_Dir/$(basename $File_Name .txt)* |head -1))
	echo "the import file exists: $Import_File_Name"
	cd $Target_Dir


	if [ $Check_Quadratura -eq 1 ]
	then
		tail -n 1 $Import_Dir/$Import_File_Name | grep '^9999'
		Check_Quadratura_Exit_Status=$?

		if [ $Check_Quadratura_Exit_Status -eq 1 ]
		then

			# Se il file non contiene quadratura e è stato richiesto
			# il controllo quadratura esco con messaggio di errore (mail)

			Mail_Subject="Errore in ricezione file [$Import_File_Name]"
			message="File in ricezione [$Import_File_Name] non contiene record di quadratura"
			echo "insert into mail_queue (mqstato,mqtoadd,mqccadd,mqrtadd,mqsubjt,mqtbody,mqattac,mqtmins,mqprior) values ('Q','albertoperusi@boxlinesrl.com','automazioneelogistica@gmail.com','albertoperusi@boxlinesrl.com','$Mail_Subject','$message','','now',20);"|psql -h database mondadori
			exit 1

		fi
	fi


#	if [ -f $File_Name ]
#	then
#		echo "the target file exists: $File_Name. Waiting for someone to receive it"
#	else
#		echo "the target file doesn't exist: making a new target file with the import file $Import_File_Name"
#		chmod a+rwx $Import_Dir/$Import_File_Name
#		cp $Import_Dir/$Import_File_Name  $Backup_Dir/$Import_File_Name
#		mv -T -f $Import_Dir/$Import_File_Name  $Target_Dir/$File_Name
#		Mv_Exit_Status=$?
#	fi

	chmod a+rwx $Import_Dir/$Import_File_Name
	cp $Import_Dir/$Import_File_Name  $Backup_Dir/$Import_File_Name
	mv -T -f $Import_Dir/$Import_File_Name  $Target_Dir/$Import_File_Name
	Mv_Exit_Status=$?



else
	echo "the import file doesn't exists"
fi


case $Mv_Exit_Status in
0)
	#Message="Creato file $File_Name da $Import_File_Name"
	Message="Creato file $Import_File_Name da $Import_File_Name"
	echo "insert into ftp_receive_log (message,code) values ('$Message',$Mv_Exit_Status)"|psql -h database mondadori 
	;;
*)
	Message="Errore generico"
	;;
esac

exit $Mv_Exit_Status 

