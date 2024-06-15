#!/bin/bash
#
# Mail_Sender.sh
# Versione: 1.0
# Autore: Luca Guerrini
#
# Esempio d'uso:
# mail_sender.sh --mailhost relay.poste.it --mailuser luca.guerrini@poste.it --mailpassword passwd 
#                --to luca.guerrini@poste.it --subject $Mail_Subject --message ”$Message”

E_XCD=66       # Errore di mancato cambio directory
NOARGS=0
E_BADARGS=65   # Errore di argomenti sbagliati

Mail_Host=""
Mail_User=""
Mail_Password=""
Mail_To=""
Many_Mail_To=""
Mail_Cc=""
Mail_Bcc=""
Mail_Message=""
Mail_Attach=""
Many_Mail_Attach=""

Current_Date=`date`

function print_help()
{
	echo
	echo "Usage: `basename $0` [options]"
	echo
	echo "List of options:"
	echo "-mailhost <host-name>: name of the host"
	echo "-mailuser <user-address>: mail user (ex: ciccio@gmail.com)"
	echo "-mailpassword <user-password>: mail password"
	echo "-to <mail-address>"
	echo "-cc <mail-address>"
	echo "-bcc <mail-address>"
	echo "-message <mail-message>"
	echo "-attach <file>"

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
		--mailuser) Mail_User=$2 ; shift ;;
		--mailpassword) Mail_Password=$2 ; shift ;;
		--mailhost) Mail_Host=$2 ; shift ;;
		--to)  Mail_To=$2 ; shift ;;
		--cc)  Mail_Cc=$2 ; shift ;;
		--bcc) Mail_Bcc=$2 ; shift ;;
		--subject) Mail_Subject=$2 ; shift ;;
		--message) Mail_Message=$2 ; shift ;;
		--attach) Mail_Attach=$2 ; shift ;;
		--*) ;;
		*)  ;;
	esac
shift
done

echo "$Mail_Host"
echo "$Mail_User"
echo "$Mail_Password"
echo "$Mail_To"
echo "$Mail_Cc"
echo "$Mail_Bcc"
echo "$Mail_Subject"
echo "$Mail_Message"
echo "$Mail_Attach"

if [[ -z $Mail_User ]] || [[ -z $Mail_Password ]] || [[ -z $Mail_Host ]] \
	|| [[ -z $Mail_To ]] || [[ -z $Mail_Subject ]] 
then
	print_help
	exit 1
fi

#if [[ -z $Mail_Attach ]]
#then
#	Attach_String=" "
#else
#	Attach_String="--attach=$Mail_Attach"
#fi

if [[ -z $Mail_Message ]]
then
	Body_String=" "
else
	Body_String="--body-plain=$Mail_Message"
fi

if [[ -z $Mail_Cc ]]
then
	Cc_String=" "
else
	Cc_String=" --cc=$Mail_Cc "
fi


## Devo fare il parsing di Mail_To, perche' se ho piu' indirizzi
## li devo mettere nella forma --to <add1> --to <add2> ....
## Cambio la variabile di sistema IFS e poi la rimetto a posto 
## In questo modo e' tutto molto semplice

OLDIFS=$IFS
IFS=,
for i in $Mail_To
do
	Many_Mail_To=$Many_Mail_To" --to "$i
done
if [[ -z $Mail_Attach ]]
then
	Many_Mail_Attach=" "
else
	for i in $Mail_Attach
	do
		Many_Mail_Attach=$Many_Mail_Attach" --attach="$i 
	done
fi
IFS=$OLDIFS

## Vecchio comando a singolo indirizzo mail
#SMTP_ERROR=$(smtp-cli --host $Mail_Host --auth-plain --user=$Mail_User --pass=$Mail_Password --from $Mail_User \
#--to $Mail_To --subject "$Mail_Subject" $Attach_String "$Body_String" 2>&1)

## Nuovo comando a indirizzo mail multiplo
SMTP_ERROR=$(smtp-cli --host $Mail_Host --auth-plain --user=$Mail_User --pass=$Mail_Password --from $Mail_User \
$Many_Mail_To $Cc_String --subject "$Mail_Subject" $Many_Mail_Attach "$Body_String" 2>&1)

#Se la stringa di errore non e' vuota allora e' successo qualcosa
if [ ! -n "$SMTP_ERROR" ]
then
	echo "Spedita mail"
	echo "insert into mail_sender_log (message,code) values ('Spedita Mail a $Many_Mail_To', 0)"|psql -h database mondadori
	exit 0
else
	echo "Mail non spedita"
	## tolgo eventuali apici che mi fanno casino nella query
	echo "insert into mail_sender_log (message,code) values ('Errore in spedizione a $Many_Mail_To: $(echo $SMTP_ERROR|tr -d \"\'\")',1)"|psql -h database mondadori
	exit 1
fi




