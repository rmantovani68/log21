#!/bin/bash
#
# ftp_send.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# ftp_send.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input  --filename /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-export/provaexport1.txt

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

Ftp_Host=""
Ftp_User=""
Ftp_Password=""
File_Name=""
Remote_Dir=""
Backup_Dir=""
Log_Dir="~"

Send_Mail=0
Current_Date=`date`

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
	echo "-logdir <dir>"
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
		--filename) File_Name=$2 ; shift ;;
		--remotedir) Remote_Dir=$2 ; shift ;;
		--backupdir) Backup_Dir=$2 ; shift ;;
		--logdir) Log_Dir=$2 ; shift ;;
		--*) ;;
		*)  ;;
	esac
shift
done

if [[ -z $Ftp_User ]] || [[ -z $Ftp_Password ]] || [[ -z $Ftp_Host ]] || [[ -z $File_Name ]] || [[ -z $Remote_Dir ]] || [[ -z $Backup_Dir ]] || [[ -z $Log_Dir ]]
then
	print_help
	exit 1
fi


First_File=$(ls -1 $File_Name | head -n 1)
echo $First_File
if [[ -z $First_File ]] 
then
	echo "Nessun file da spedire"
	exit 0
fi

echo "ncftpput -u $Ftp_User -p $Ftp_Password -d $Log_Dir/ftp_send.log -t 300 -T tmp_ $Ftp_Host $Remote_Dir $First_File"

ncftpput -u $Ftp_User -p $Ftp_Password -d $Log_Dir/ftp_send.log -t 300 -T tmp_ $Ftp_Host $Remote_Dir $First_File
Ftp_Exit_Status=$?

case $Ftp_Exit_Status in
0)
	Send_Mail=1
	Mail_Subject="Spedito file $First_File"
	Message="Spedito file $First_File"
	mv $First_File $Backup_Dir
	;;
1 | 2)
	Message="Impossibile connettersi al server"
	;;
3 | 4)
	Message="Errore in trasferimento file"
	;;
5 | 6)
	Message="Impossibile cambiare directory"
	;;
7)
	Message="URL malformato"
	;;
*)
	Message="Errore generico"
	;;
esac

echo $Message
echo "insert into ftp_send_log (message,code) values ('$Message',$Ftp_Exit_Status)"|psql -h database mr 

if [ $Send_Mail -eq 1 ]
then
	#/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita --to albertoperusi@boxlinesrl.com --subject "$Mail_Subject" --message "$Message"
	echo "insert into mail_queue (mqstato,mqtoadd,mqccadd,mqrtadd,mqsubjt,mqtbody,mqattac,mqtmins,mqprior) values ('Q','albertoperusi@boxlinesrl.com','automazioneelogistica@gmail.com','albertoperusi@boxlinesrl.com','$Mail_Subject','$message','','now',20);"|psql -h database mondadori
fi

if [ $Ftp_Exit_Status -eq 0 ]
then
	exit 0
else
	exit 1
fi

#ncftpls -u mpf-boxline -p eonrc44 ftp://ftp.mondadori.it/output
