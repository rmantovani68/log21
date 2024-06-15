#!/bin/bash
#
# statistica-giornaliera.sh
# Versione: 1.0
# Autore: Roberto Mantovani
#

Current_Date=`date`
TIME_STAMP=`date +%Y%m%d%H%M%S` 
Mail_Subject="statistica giornaliera $Current_Date"
Message="In Allegato Statistica giornaliera del $Current_Date"
Attach=/u/prj/mondadori-rcs-ufficio/sql/statistica-giornaliera.txt 

psql -h database mr < /u/prj/mondadori-rcs-ufficio/sql/crea-statistica-giornaliera.sql 
psql -h database mr < /u/prj/mondadori-rcs-ufficio/sql/select-statistica-giornaliera.sql > /u/prj/mondadori-rcs-ufficio/sql/statistica-giornaliera.txt 
/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita --to rmantovani68@gmail.com --subject "$Mail_Subject" --message "$Message" --attach "$Attach"
/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita --to alberto.perusi@booksrv.it --subject "$Mail_Subject" --message "$Message" --attach "$Attach"
/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita --to albertoperusi@boxlinesrl.com --subject "$Mail_Subject" --message "$Message" --attach "$Attach"
#/usr/local/bin/mail_sender.sh --mailhost smtp.boxlinesrl.com --mailuser flussodatinovita@boxlinesrl.com --mailpassword box@flussodatinovita --to alberto.perusi@booksrv.it --subject "$Mail_Subject" --message "$Message" --attach "$Attach"
