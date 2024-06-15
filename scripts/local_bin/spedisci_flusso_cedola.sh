#!/bin/bash

# spedisci_flusso_cedola.sh

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

export_ordini_filename=LA_BOLLE.TXT
export_righe_filename=LA_RIGHE_COLLI.TXT

export_ordini_filename_TS=LA_BOLLE_$TIME_STAMP.TXT
export_righe_filename_TS=LA_RIGHE_COLLI_$TIME_STAMP.TXT

# controllo esistenza files
if [[ -f $export_dir/$export_ordini_filename ]] || [[ -f $export_dir/$export_righe_filename ]] 
then
	echo "export_righe_filename :"$export_dir/$export_righe_filename
	echo "export_ordini_filename:"$export_dir/$export_ordini_filename
else
	echo "I files da trasmettere non sono presenti"
	echo "export_righe_filename :"$export_dir/$export_righe_filename
	echo "export_ordini_filename:"$export_dir/$export_ordini_filename
	exit 1
fi


# rinomino i files di export
echo "rinomino i files di export"
mv  $export_dir/$export_ordini_filename $export_dir/$export_ordini_filename_TS
mv  $export_dir/$export_righe_filename  $export_dir/$export_righe_filename_TS

# controllo rinomina eseguita
if [[ -f $export_dir/$export_ordini_filename_TS ]] || [[ -f $export_dir/$export_righe_filename_TS ]] 
then
	echo "export_righe_filename_TS :"$export_dir/$export_righe_filename_TS
	echo "export_ordini_filename_TS:"$export_dir/$export_ordini_filename_TS
else
	echo "I files da trasmettere (TS) non sono presenti"
	echo "export_righe_filename_TS :"$export_dir/$export_righe_filename_TS
	echo "export_ordini_filename_TS:"$export_dir/$export_ordini_filename_TS
	exit 1
fi


# sposto i files di export

#/usr/local/bin/move_export_file_to_send.sh --filename $export_righe_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir >/dev/null 2>&1
#/usr/local/bin/move_export_file_to_send.sh --filename $export_ordini_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir >/dev/null 2>&1
#/usr/local/bin/move_export_file_to_send.sh --filename $export_righe_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir 
#/usr/local/bin/move_export_file_to_send.sh --filename $export_ordini_filename_TS  --exportdir $export_dir --targetdir $ftp_export_dir 

echo mv $export_dir/$export_ordini_filename_TS $ftp_export_dir/$export_ordini_filename_TS
echo mv $export_dir/$export_righe_filename_TS  $ftp_export_dir/$export_righe_filename_TS
mv $export_dir/$export_ordini_filename_TS $ftp_export_dir/$export_ordini_filename_TS
mv $export_dir/$export_righe_filename_TS  $ftp_export_dir/$export_righe_filename_TS

# controllo spostamento eseguita
if [[ -f $ftp_export_dir/$export_ordini_filename_TS ]] || [[ -f $ftp_export_dir/$export_righe_filename_TS ]] 
then
	echo "export_righe_filename_TS :"$ftp_export_dir/$export_righe_filename_TS
	echo "export_ordini_filename_TS:"$ftp_export_dir/$export_ordini_filename_TS
else
	echo "I files da trasmettere (TS) non sono presenti"
	echo "export_righe_filename_TS :"$ftp_export_dir/$export_righe_filename_TS
	echo "export_ordini_filename_TS:"$ftp_export_dir/$export_ordini_filename_TS
	exit 1
fi

# spedisco i files di export
#/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --filename $export_ordini_filename_TS >/dev/null 2>&1
#/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --filename $export_righe_filename_TS >/dev/null 2>&1
/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --filename $ftp_export_dir/$export_ordini_filename_TS 
/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --filename $ftp_export_dir/$export_righe_filename_TS 
exit 0
