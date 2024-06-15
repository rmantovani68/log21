#!/bin/bash
DATABASE_NAME=mr
DATABASE_HOST=$HOSTNAME
TRACE_FILE=/u/prj/mondadori-rcs-ufficio/trace/root_monthly_scripts.trc
EXPORT_PATH=/u/prj/mondadori-rcs-ufficio/export
BACKUP_EXPORT_PATH=/u/prj/mondadori-rcs-ufficio/export/backup
DATE=`date +%Y%m%d`





echo -n > $TRACE_FILE
#xhost>> $TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Spostamento file export piu' vecchi di 30 gg in export/backup" >> $TRACE_FILE
cd $EXPORT_PATH
find $EXPORT_PATH -type f -name '*' -mtime +30 -exec mv {} $BACKUP_EXPORT_PATH \;
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Archiviazione file in export/backup" >> $TRACE_FILE
cd $BACKUP_EXPORT_PATH
tar zcf backup_dati_export.tgz *
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Spostamento archivio da export/backup a /home/delta/backup" >> $TRACE_FILE
cd $BACKUP_EXPORT_PATH
mv $BACKUP_EXPORT_PATH/backup_dati_export.tgz /home/alberto/backup/backup_dati_export_$DATE.tgz
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
# echo  "Cancellazione file export in export/backup" >> $TRACE_FILE
# cd $BACKUP_EXPORT_PATH
# rm -f $BACKUP_EXPORT_PATH/*
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

