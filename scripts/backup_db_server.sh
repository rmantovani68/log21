#!/bin/bash
gdialog --title "Backup Database" --yesno "Confermi il backup del DataBase ?" 350 350
#echo $?
case $? in
	0) 
		rsh database 'cd /u/prj/mondadori/backup;pg_dump mondadori|gzip > mondadori.dump.gz'
	;;
	1) 
		:
	;;
esac
