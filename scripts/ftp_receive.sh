#!/bin/bash
#
# ftp_receive.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso
# ftp_receive.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it/output 
#             --localdir /u/prj/mondadori-ufficio-luca/export/ftp-mondadori/ --filename cacni*

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

Ftp_Host=""
Ftp_User=""
Ftp_Password=""
File_Name=""
Mail_Notification="false"
Local_Dir=""
First_File=""

Send_Mail=0
Current_Date=`date`
Now_String=$(date +"%Y%m%d%H%M%S")

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
	echo "-localdirc <dir>"
	echo "-mailnotification"
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
		--localdir) Local_Dir=$2 ; shift ;;
		--mailnotification) Mail_Notification="true" ;;
		--*) ;;
		*)  ;;
	esac
shift
done

if [[ -z $Ftp_User ]] || [[ -z $Ftp_Password ]] || [[ -z $Ftp_Host ]] \
	|| [[ -z $File_Name ]] || [[ -z $Local_Dir ]] 
then
	print_help
	exit 1
fi

cd $Local_Dir

echo "Nome File Inserito:"
echo $File_Name
First_File=$(ncftpls -u mpf-boxline -p eonrc44  ftp://ftp.mondadori.it/output/$File_Name | head -n 1)
echo $First_File
if [[ -z $First_File ]] 
then
	echo "Nessun file da trasferire"
	exit 0
fi


# Attenzione, qui sotto -DD per cancellare i file a fine scaricamento
#ncftpget -u $Ftp_User -p $Ftp_Password -t 300 $Ftp_Host $Local_Dir $File_Name
#echo "ncftpget -u $Ftp_User -p $Ftp_Password -t 300 $Ftp_Host $Local_Dir $File_Name"
echo "ncftpget -u $Ftp_User -p $Ftp_Password -t 300 ftp://$Ftp_Host/$First_File"

ncftpget -u $Ftp_User -p $Ftp_Password -t 300 -DD ftp://$Ftp_Host/$First_File
Ftp_Exit_Status=$?

case $Ftp_Exit_Status in
0)
	Send_Mail=1
	Mail_Subject="Ricevuto file $First_File"
	Message="Ricevuto file $First_File"
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
echo "insert into ftp_receive_log (message,code) values ('$Message',$Ftp_Exit_Status)"|psql -h database mondadori 

if [ "$Mail_Notification"=="true" ]
then
	if [ $Send_Mail -eq 1 ]
	then
		#/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita \
		#	--to albertoperusi@boxlinesrl.com --subject "$Mail_Subject" --message "$Message"
		#echo "ive_log (message,code) values ('$Message',$Ftp_Exit_Status)"|psql -h database mondadori 
		echo "insert into mail_queue (mqstato,mqtoadd,mqccadd,mqrtadd,mqsubjt,mqtbody,mqattac,mqtmins,mqprior) values ('Q','albertoperusi@boxlinesrl.com','automazioneelogistica@gmail.com','albertoperusi@boxlinesrl.com','$Mail_Subject','$message','','now',20);"|psql -h database mondadori
	fi
fi

if [ $Ftp_Exit_Status -eq 0 ]
then
	exit 0
else
	Error_File_Name=error_$File_Name_$Now_String
	mv $First_File $Error_File_Name
	exit 1
fi

#ncftpput -u mpf-boxline -p eonrc44 -t 300 ftp.mondadori.it output cacni*
#ncftpls -u mpf-boxline -p eonrc44 ftp://ftp.mondadori.it/output
