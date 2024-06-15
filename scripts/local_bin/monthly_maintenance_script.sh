#!/bin/bash
DATABASE_NAME=mondadori
DATABASE_HOST=database
TRACE_FILE=/u/prj/mondadori/trace/monthly_maintenance_scripts.trc
DATE=`date +%Y%m%d`

echo -n > $TRACE_FILE
#xhost>> $TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Spostamento file corriere piu' vecchi di 30 gg in storico/file-corriere/backup" >> $TRACE_FILE
cd /u/prj/mondadori/export/file-corriere-backup
find /u/prj/mondadori/export/file-corriere-backup -type f -name '*' -mtime +30 -exec mv {} /u/prj/mondadori/storico/file-corriere/backup \;
cd /u/prj/mondadori/export/file-corriere
find /u/prj/mondadori/export/file-corriere -type f -name '*' -mtime +30 -exec mv {} /u/prj/mondadori/storico/file-corriere/backup \;
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Archiviazione file in storico/file-corriere/backup" >> $TRACE_FILE
cd /u/prj/mondadori/storico/file-corriere/backup
tar zcf backup_file_corriere.tgz *
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Spostamento archivio da storico/file-corriere/backup a storico/file-corriere" >> $TRACE_FILE
cd /u/prj/mondadori/storico/file-corriere/backup
mv /u/prj/mondadori/storico/file-corriere/backup/backup_file_corriere.tgz /u/prj/mondadori/storico/file-corriere/backup_file_corriere_$DATE.tgz
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "Cancellazione file export in storico/file-corriere/backup" >> $TRACE_FILE
#cd /u/prj/mondadori/storico/file-corriere/backup
#rm -f /u/prj/mondadori/storico/file-corriere/backup/*
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

