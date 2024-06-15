#!/bin/bash

#
# spostamento file e spedizione via ftp a server 
#

TIME_STAMP=`date +%Y%m%d%H%M%S` 

export_dir=/u/prj/mondadori/export
ftp_export_dir=/u/prj/mondadori/export/ftp-mondadori-export

export_ordini_filename=conscni.txt

# sposto i files di export
/usr/local/bin/move_export_file_to_send.sh --filename $export_ordini_filename  --exportdir $export_dir --targetdir $ftp_export_dir >/dev/null 2>&1

# spedisco i files di export
/usr/local/bin/ftp_send.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input --backupdir /u/prj/mondadori/export/ftp-mondadori-export-backup  --filename /u/prj/mondadori/export/ftp-mondadori-export/conscni* >/dev/null 2>&1
exit 0
