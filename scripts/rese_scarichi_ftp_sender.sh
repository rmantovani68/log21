#!/bin/bash
#
# ftp_sender.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# ftp_sender.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input  --filename /u/prj/mondadori-ufficio-luca/export/ftp-mondadori-export/provaexport1.txt

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

Ftp_Host=""
Ftp_User=""
Ftp_Password=""
File_Name=""
Remote_Dir=""
Backup_Dir=""

#Send_Mail=0
#Current_Date=`date`
#
#function print_help()
#{
#	echo
#	echo "Usage: `basename $0` [options]"
#	echo
#	echo "List of options:"
#	echo "-ftphost <host-name>: name of the host"
#	echo "-ftpuser <user-address>: ftp user"
#	echo "-ftppassword <user-password>: ftp password"
#	echo "-filename <file>"
#	echo "-remotedir <dir>"
#	echo "-backupdir <dir>"
#	echo
#}
#
#
#if [ $# = $NOARGS ]
#then
#	print_help
#	exit $E_BADARGS
#fi
#
#while [ $# != 0 ]
#do
#	case "$1" in
#		--ftpuser) Ftp_User=$2 ; shift ;;
#		--ftppassword) Ftp_Password=$2 ; shift ;;
#		--ftphost) Ftp_Host=$2 ; shift ;;
#		--filename) File_Name=$2 ; shift ;;
#		--remotedir) Remote_Dir=$2 ; shift ;;
#		--backupdir) Backup_Dir=$2 ; shift ;;
#		--*) ;;
#		*)  ;;
#	esac
#shift
#done
#
#if [[ -z $Ftp_User ]] || [[ -z $Ftp_Password ]] || [[ -z $Ftp_Host ]] \
#	|| [[ -z $File_Name ]] || [[ -z $Remote_Dir ]] || [[ -z $Backup_Dir ]]
#then
#	print_help
#	exit 1
#fi



cd /u/prj/deltastock/export/rese_flussi_clienti
FN=$(find . -iregex './scarico[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].txt'| head -1)
echo $FN
File_Name=$(basename $FN)
echo "file name: [$File_Name]"

if [ -e "$File_Name" ]
then
	echo "Tento di spedire via ftp $File_Name"
  /usr/local/bin/ftp_sender.sh --ftpuser boxline --ftppassword boxline --ftphost ftp.mondadori-franchising.it --remotedir . --filename /u/prj/deltastock/export/rese_flussi_clienti/$File_Name --backupdir /u/prj/deltastock/export/rese_flussi_clienti/backup
#  ftp_sender.sh --ftpuser delta --ftppassword albaro --ftphost ta_albaro --remotedir /home/delta/prove-spedizione --filename /u/prj/deltastock/export/rese_flussi_clienti/$File_Name --backupdir /u/prj/deltastock/export/rese_flussi_clienti/backup
else
	echo "Nessun File Trovato"
fi

exit 0

#
#echo $Message
#echo "insert into ftp_sender_log (message,code) values ('$Message',$Ftp_Exit_Status)"|psql -h database mondadori 
#
#if [ $Send_Mail -eq 1 ]
#then
#	/usr/local/bin/mail_sender.sh --mailhost smtp.deltatradingsrl.com --mailuser albertoperusi@deltatradingsrl.com --mailpassword delta@albertop --to albertoperusi@deltatradingsrl.com --subject "$Mail_Subject" --message "$Message"
#fi
#
#if [ $Ftp_Exit_Status -eq 0 ]
#then
#	exit 0
#else
#	exit 1
#fi
#
##ncftpls -u mpf-boxline -p eonrc44 ftp://ftp.mondadori.it/output
