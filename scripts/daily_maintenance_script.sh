#!/bin/bash
DATABASE_NAME=mondadori
DATABASE_HOST=database
TRACE_FILE=/u/prj/mondadori/trace/daily_maintenance_scripts.trc

PATH=$PATH:/usr/local/bin
LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib:/usr/local/bin
DISPLAY=:0.0

export DISPLAY LIBRARY_PATH PATH
xhost + 

echo -n > $TRACE_FILE
#xhost>> $TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "vacuumdb" >> $TRACE_FILE
vacuumdb -f -z -h $DATABASE_HOST -d $DATABASE_NAME
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE
#
#echo -n "$(date) <---- " >> $TRACE_FILE
#echo  "REINDEX TABLE ORD_PROD" >> $TRACE_FILE
#echo -n 'REINDEX TABLE ORD_PROD;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
#echo -n " ---> $(date)" >> $TRACE_FILE
#
#echo >>$TRACE_FILE
#echo >>$TRACE_FILE
#
#echo -n "$(date) <---- " >> $TRACE_FILE
#echo  "START RECEIVE 6" >> $TRACE_FILE
#cd /u/prj/deltastock/exe
#./receive -a -p 6 &
#echo -n " ---> $(date)" >> $TRACE_FILE
#
#echo >>$TRACE_FILE
#echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "insert into registro_mail_corriere_stor select * from registro_mail_corriere where rmprogr=(select max(rmprogr) from registro_mail_corriere)-5000;" >> $TRACE_FILE
echo -n 'insert into registro_mail_corriere_stor select * from registro_mail_corriere where rmprogr<=(select max(rmprogr) from registro_mail_corriere)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "delete from registro_mail_corriere where rmprogr<=(select max(rmprogr) from registro_mail_corriere)-5000;" >> $TRACE_FILE
echo -n 'delete from registro_mail_corriere where rmprogr<=(select max(rmprogr) from registro_mail_corriere)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "insert into ftp_receive_log_stor select * from ftp_receive_log where id=(select max(id) from ftp_receive_log)-5000;" >> $TRACE_FILE
echo -n 'insert into ftp_receive_log_stor select * from ftp_receive_log where id<=(select max(id) from ftp_receive_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "delete from ftp_receive_log where id<=(select max(id) from ftp_receive_log)-5000;" >> $TRACE_FILE
echo -n 'delete from ftp_receive_log where id<=(select max(id) from ftp_receive_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "insert into ftp_send_log_stor select * from ftp_send_log where id=(select max(id) from ftp_send_log)-5000;" >> $TRACE_FILE
echo -n 'insert into ftp_send_log_stor select * from ftp_send_log where id<=(select max(id) from ftp_send_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "delete from ftp_send_log where id<=(select max(id) from ftp_send_log)-5000;" >> $TRACE_FILE
echo -n 'delete from ftp_send_log where id<=(select max(id) from ftp_send_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "insert into mail_sender_log_stor select * from mail_sender_log where id=(select max(id) from mail_sender_log)-5000;" >> $TRACE_FILE
echo -n 'insert into mail_sender_log_stor select * from mail_sender_log where id<=(select max(id) from mail_sender_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

echo -n "$(date) <---- " >> $TRACE_FILE
echo  "delete from mail_sender_log where id<=(select max(id) from mail_sender_log)-5000;" >> $TRACE_FILE
echo -n 'delete from mail_sender_log where id<=(select max(id) from mail_sender_log)-5000;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
echo -n " ---> $(date)" >> $TRACE_FILE

echo >>$TRACE_FILE
echo >>$TRACE_FILE

## Storicizzo gli udc spediti da piu' di 30 giorni
#
#echo -n "$(date) <---- " >> $TRACE_FILE
#echo  "insert into prodotti_udc_spediti select * from prodotti_udc where pucdudc in (select uccdudc from udc where uctmspe<(CURRENT_DATE-30));" >> $TRACE_FILE
#echo -n 'insert into prodotti_udc_spediti select * from prodotti_udc where pucdudc in (select uccdudc from udc where uctmspe<(CURRENT_DATE-30));' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
#echo -n " ---> $(date)" >> $TRACE_FILE
#
#echo >>$TRACE_FILE
#echo >>$TRACE_FILE
#
#echo -n "$(date) <---- " >> $TRACE_FILE
#echo  "delete from prodotti_udc where pucdudc in (select uccdudc from udc where uctmspe<(CURRENT_DATE-30));" >> $TRACE_FILE
#echo -n 'delete from prodotti_udc where pucdudc in (select uccdudc from udc where uctmspe<(CURRENT_DATE-30));' | psql -h $DATABASE_HOST $DATABASE_NAME >> $TRACE_FILE 2>&1
#echo -n " ---> $(date)" >> $TRACE_FILE
#
#echo >>$TRACE_FILE
#echo >>$TRACE_FILE

#echo -n "$(date) <---- " >> $TRACE_FILE
#echo  "START PRINTLABEL 5" >> $TRACE_FILE
#cd /u/prj/deltastock/exe
#./printlabel -p 5 &
#echo -n " ---> $(date)" >> $TRACE_FILE
