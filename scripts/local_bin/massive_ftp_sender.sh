#!/bin/bash
#
# ftp_sender_2.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# ./massive_ftp_sender.sh --ftpuser luca --ftppassword 3zin --ftphost 192.168.1.43 --remotedir /home/luca/output  --dbhost localhost --dbname mondadori --localdir /u/prj/mondadori-ufficio/export/pdf --backupdir /u/prj/mondadori-ufficio/export/pdf/backup

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

DB_Host=""
DB_Name=""
Ftp_Host=""
Ftp_User=""
Ftp_Password=""
File_Name=""
Remote_Dir=""
Backup_Dir=""
Local_Dir=""

Send_Mail=0
Current_Date=`date`

Exit_Safely()
{
	cd $Local_Dir
	rm -f .PID
	exit 2
}

trap Exit_Safely INT TERM EXIT

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "-ftphost <host-name>: name of the host"
	echo "-ftpuser <user-address>: ftp user"
	echo "-ftppassword <user-password>: ftp password"
	echo "-filename <file>"
	echo "-remotedir <dir>"
	echo "-backupdir <dir>"
	echo "-localdir <dir>"
	echo "-dbhost <DB-host>: name of the DB host"
	echo "-dbname <DB-name>: name of the DB"
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
		--ftpuser) Ftp_User=$2 ; shift ;;
		--ftppassword) Ftp_Password=$2 ; shift ;;
		--ftphost) Ftp_Host=$2 ; shift ;;
		--localdir) Local_Dir=$2 ; shift ;;
		--remotedir) Remote_Dir=$2 ; shift ;;
		--backupdir) Backup_Dir=$2 ; shift ;;
		--dbhost) DB_Host=$2 ; shift ;;
		--dbname) DB_Name=$2 ; shift ;;
		--*) ;;
		*)  ;;
	esac
shift
done

if [[ -z $Ftp_User ]] || [[ -z $Ftp_Password ]] || [[ -z $Ftp_Host ]] \
	|| [[ -z $Remote_Dir ]] || [[ -z $Backup_Dir ]] || [[ -z $Local_Dir ]] \
	|| [[ -z $DB_Host ]] || [[ -z $DB_Name ]]
then
	print_help
	exit 1
fi


cd $Local_Dir

if [ -f ".PID" ]
then
	echo "PID presente. Non faccio nulla"
	exit 1
else
	echo "1" > .PID
fi


for First_File in `find -maxdepth 1 -type f ! -name '*\.part' ! -name '\.PID' -print`
do
	if [[ -d $First_File ]] 
	then
		echo "Directory: non spedisco"
	else
		echo "ncftpput -u $Ftp_User -p $Ftp_Password -t 300 -S .part $Ftp_Host $Remote_Dir $First_File"

		ncftpput -u $Ftp_User -p $Ftp_Password -t 1200 -S .part $Ftp_Host $Remote_Dir $First_File
		Ftp_Exit_Status=$?

		case $Ftp_Exit_Status in
		0)
			echo $First_File
			Send_Mail=1
			Mail_Subject="FILE BOXLINE: $First_File"
			Message="Spedito file $First_File"
			echo $Message
			;;
		1 | 2)
			Mail_Subject="FILE BOXLINE: Impossibile connettersi al server"
			Message="Impossibile connettersi al server"
			;;
		3 | 4)
			Mail_Subject="FILE BOXLINE: Errore in trasferimento file"
			Message="Errore in trasferimento file"
			;;
		5 | 6)
			Mail_Subject="FILE BOXLINE: Impossibile cambiare directory"
			Message="Impossibile cambiare directory"
			;;
		7)
			Mail_Subject="FILE BOXLINE: URL malformato"
			Message="URL malformato"
			;;
		*)
			Mail_Subject="FILE BOXLINE: Errore generico"
			Message="Errore generico"
			;;
		esac

		echo $Message
		echo "insert into ftp_sender_log (message,code) values ('$Message',$Ftp_Exit_Status)"|psql -h $DB_Host $DB_Name


		if [ $Send_Mail -eq 1 ]
		then
			echo "Mando mail"
		fi

		if [ $Ftp_Exit_Status -eq 0 ]
		then
			mv $First_File $Backup_Dir
		else
			echo "Errore FTP"
		fi
	fi
done

rm .PID

exit 0

