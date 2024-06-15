#!/bin/bash

# spedisci_consuntivo_ceva.sh

#
# spostamento file e spedizione via ftp a server 
#

TIME_STAMP=`date +%Y%m%d%H%M%S` 

export_dir=/u/prj/mondadori-rcs-ufficio/export
ftp_export_dir=/u/prj/mondadori-rcs-ufficio/ftp-mondadori-export

export_ordini_filename=LA_BOLLE.TXT
export_righe_filename=LA_RIGHE_COLLO.TXT

export_ordini_filename_TS=LA_BOLLE_$TIME_STAMP.TXT
export_righe_filename_TS=LA_RIGHE_COLLO_$TIME_STAMP.TXT

echo 'export_righe_filename_TS ='$export_righe_filename_TS
echo 'export_ordini_filename_TS='$export_ordini_filename_TS

# rinomino i files di export
mv  $export_dir/$export_ordini_filename $export_dir/$export_ordini_filename_TS
mv  $export_dir/$export_righe_filename $export_dir/$export_righe_filename_TS

# sposto i files di export
/usr/local/bin/move_export_file_to_send.sh --filename $export_righe_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir >/dev/null 2>&1
/usr/local/bin/move_export_file_to_send.sh --filename $export_ordini_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir >/dev/null 2>&1

# spedisco i files di export
/usr/local/bin/ftp_send.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input --backupdir /u/prj/mondadori/export/ftp-mondadori-export-backup  --filename /u/prj/mondadori/export/ftp-mondadori-export/conscni* >/dev/null 2>&1
/usr/local/bin/ftp_send.sh --ftpuser mpf-boxline --ftppassword eonrc44 --ftphost ftp.mondadori.it --remotedir input --backupdir /u/prj/mondadori/export/ftp-mondadori-export-backup  --filename /u/prj/mondadori/export/ftp-mondadori-export/conscni* >/dev/null 2>&1
exit 0
