#!/bin/bash

# spedisci_consuntivo_ceva.sh

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
ftp_log_dir=/u/prj/mondadori-rcs-ufficio/export/ftp-log

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
/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --logdir $ftp_log_dir --filename $ftp_export_dir/$export_ordini_filename_TS 
/usr/local/bin/ftp_send.sh --ftpuser $ftp_user --ftppassword $ftp_pwd --ftphost $ftp_host --remotedir $ftp_remote_dir --backupdir $ftp_export_backup_dir  --logdir $ftp_log_dir --filename $ftp_export_dir/$export_righe_filename_TS 
exit 0
