#!/bin/bash

# spedisci_rimanenze_ceva.sh

#
# spostamento file e spedizione via ftp a server 
#

TIME_STAMP=`date +%Y%m%d%H%M%S` 

ftp_host=ftpext.cevalogistics.com
ftp_user=edi_mbxln_prd
ftp_pwd=eD1Mbxln@pr0d
ftp_remote_dir=to_ceva

export_dir=/u/prj/mondadori-rcs-ufficio/export
ftp_export_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-mondadori-export
ftp_export_backup_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-mondadori-export-backup

for export_file_path in $(ls $export_dir/BL_*.TXT)  
do
	file_name="$(basename $export_file_path)"
	echo "file :"$file_name
	# sposto i files da export alla cartella ftp-export
	mv $export_dir/$file_name  $ftp_export_dir/$file_name

	# controllo spostamento eseguito
	if [[ -f $ftp_export_dir/$file_name ]] 
	then
		echo "export_rimanenze:"$ftp_export_dir/$file_name
	else
		echo "Il file da trasmettere [$ftp_export_dir/$file_name] non è presente"
		exit 1
	fi
	# spedisco i files in ftp-export
	/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --filename $ftp_export_dir/$file_name 

done
