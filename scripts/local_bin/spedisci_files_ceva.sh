#!/bin/bash

# spedisci_files_ceva.sh

#
# spedizione via ftp a server 
#

TIME_STAMP=`date +%Y%m%d%H%M%S` 

ftp_host=ftpext.cevalogistics.com
ftp_user=edi_mbxln_prd
ftp_pwd=eD1Mbxln@pr0d
ftp_remote_dir=to_ceva

export_dir=/u/prj/mondadori-rcs-ufficio/export
ftp_export_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-mondadori-export
ftp_export_backup_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-mondadori-export-backup
ftp_log_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-log

for export_file_path in $(ls $ftp_export_dir/*.TXT)  
do
	file_name="$(basename $export_file_path)"
	echo "file :"$file_name

	# spedisco i files in ftp-export
	/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir --logdir $ftp_log_dir --filename $ftp_export_dir/$file_name 
done
