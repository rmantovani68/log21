#!/bin/bash
#
# receive_ceva.sh
# Versione: 1.0
# Autore: Roberto Mantovani
#
# Esempio d'uso
# receive_ceva.sh --importdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-import --targetdir /u/prj/mondadori-ufficio-luca/export

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

File_Name=""
Import_Dir=""
Backup_Dir=""
Target_Dir=""

Current_Date=`date`
TIME_STAMP=`date +%Y%m%d%H%M%S` 

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "--importdir <dir>"
	echo "--targetdir <dir>"
	echo "--backupdir <dir>"
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
if [[ -z $Import_Dir ]] || [[ -z $Target_Dir ]] || [[ -z $Backup_Dir ]]
then
	print_help
	exit 1
fi


# devo trovare LA_TESTATE_*.TXT e LA_RIGHE_*.TXT e LA_CATALOGO_*.TXT

# creo un file LA_TESTATE.TXT contenente tutti i files testata
# e un file LA_RIGHE.TXT contenente tutti i files righe
# e un file LA_CATALOGO.TXT contenente tutti i files catalogo

for i in $(ls -1 $Import_Dir/LA_TESTATE_*.TXT)  ; do echo $i; cat $i >> $Import_Dir/LA_TESTATE.TXT;  mv $i $Backup_Dir; done
for i in $(ls -1 $Import_Dir/LA_RIGHE_*.TXT)    ; do echo $i; cat $i >> $Import_Dir/LA_RIGHE.TXT;    mv $i $Backup_Dir; done
for i in $(ls -1 $Import_Dir/LA_CATALOGO_*.TXT) ; do echo $i; cat $i >> $Import_Dir/LA_CATALOGO.TXT; mv $i $Backup_Dir; done
for i in $(ls -1 $Import_Dir/LA_CARICHI_*.TXT)  ; do echo $i; cat $i >> $Import_Dir/LA_CARICHI.TXT;  mv $i $Backup_Dir; done

for Import_File_Name in $(echo "LA_TESTATE.TXT LA_RIGHE.TXT LA_CATALOGO.TXT LA_CARICHI.TXT")

	do
	echo "file :"$Import_File_Name

	if [ -f $Import_Dir/$Import_File_Name ]
	then
		echo "the import file exists: $Import_Dir/$Import_File_Name"

		chmod a+rwx $Import_Dir/$Import_File_Name
		echo cp $Import_Dir/$Import_File_Name  $Backup_Dir/$Import_File_Name'_'$TIME_STAMP
		cp $Import_Dir/$Import_File_Name  $Backup_Dir/$Import_File_Name'_'$TIME_STAMP
		mv -T -f $Import_Dir/$Import_File_Name  $Target_Dir/$Import_File_Name
		Mv_Exit_Status=$?
		case $Mv_Exit_Status in
		0)
			Message="Creato file $Target_Dir/$Import_File_Name"
			;;
		*)
			Message="$Import_Dir/$Import_File_Name : Errore generico"
			;;
		esac

		echo "insert into ftp_receive_log (message,code) values ('$Message',$Mv_Exit_Status)" | psql -h database mr 
	else
		echo "the import file [" $Import_Dir/$Import_File_Name "] doesn't exists"
	fi

done

# creo il monitor per la ricezione automatica dei carichi

Import_File_Name="LA_CARICHI.TXT"
if [ -f $Target_Dir/$Import_File_Name ]
then
		echo $TIME_STAMP > $Target_Dir/ricezione-carichi.mon
fi

# creo il monitor per la ricezione automatica del catalogo

Import_File_Name="LA_CATALOGO.TXT"
if [ -f $Target_Dir/$Import_File_Name ]
then
		echo $TIME_STAMP > $Target_Dir/ricezione-catalogo.mon
fi
