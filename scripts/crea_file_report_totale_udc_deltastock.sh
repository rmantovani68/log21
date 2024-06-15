#!/bin/bash
#
# rese_crea_file_bancali_da_spedire.sh
# Versione: 1.0
# Autore: Luca Guerrini
#



#if [ -e "$File_Name" ]
#then
#echo "Tento di spedire via ftp $File_Name"
#/usr/local/bin/ftp_sender.sh --ftpuser boxline --ftppassword boxline --ftphost ftp.mondadori-franchising.it --remotedir . --filename /u/prj
##  ftp_sender.sh --ftpuser delta --ftppassword albaro --ftphost ta_albaro --remotedir /home/delta/prove-spedizione --filename /u/prj/deltasto
#else
#echo "Nessun File Trovato"
#fi


DATE=`date +%Y-%m-%d`
CSV_NAME=/u/prj/delta-carichi/export/file_report_totale_udc_deltastock_$DATE.csv
FILE_NAME=/u/prj/delta-carichi/export/file_report_totale_udc_deltastock_$DATE.xls
ZIP_NAME=/u/prj/delta-carichi/export/file_report_totale_udc_deltastock_$DATE.zip
echo $FILE_NAME

#psql deltastock < /u/prj/deltastock/system-sql/rese_crea_file_specchietto_lotto_21.sql > $FILE_NAME
#psql -q deltastock < /u/prj/deltastock/system-sql/giacenze_baldini.sql > $FILE_NAME
psql -q deltastock < /u/prj/delta-carichi/system-sql/report_totale_udc_deltastock.sql 
#echo "copy tmp_giacenze_societa to '$CSV_NAME' with delimiter ',' csv header quote '\"' FORCE QUOTE DEP, UBICAZIONE, UDC, EDIT, PRODOTTO, TITOLO, AUTORE, CLASS ;" | psql deltastock
echo "copy report_totale_udc to '$CSV_NAME' with delimiter ',' csv header quote '\"' force quote udc,ubicazione;" | psql -h database mondadori


/usr/local/bin/csv2xls.pl $CSV_NAME $FILE_NAME

cd /u/prj/delta-carichi/export/

rm -f $ZIP_NAME
/usr/bin/zip `basename $ZIP_NAME` `basename $FILE_NAME` `basename $CSV_NAME`


#/usr/bin/smtp-cli --verbose --server=relay.poste.it:465 --from=luca.guerrini@poste.it --to=lucaguerrini74@gmail.com --user=luca.guerrini@poste.it --pass=sesamino --auth-plain --missing-modules-ok --ssl --subject="FILE BOXLINE: $ZIP_NAME" --body-plain="In allegato: $ZIP_NAME" --attach="$ZIP_NAME@application/zip"
#/usr/bin/smtp-cli --verbose --server=relay.poste.it:465 --from=luca.guerrini@poste.it --to=lucaguerrini74@gmail.com --user=luca.guerrini@poste.it --pass=sesamino --auth-plain --missing-modules-ok --ssl --subject="FILE BOXLINE: $FILE_NAME" --body-plain="In allegato: $FILE_NAME" --attach="$FILE_NAME@application/excel"
#/usr/bin/smtp-cli --verbose --server=smtp.boxlinesrl.com --from=info@boxlinesrl.com --to=enniogia@gmail.com   --to=magazzino@boxlinesrl.com  --to=micheleperusi@boxlinesrl.com --user=info@boxlinesrl.com --pass=box@info --missing-modules-ok --subject="FILE BOXLINE: $FILE_NAME" --body-plain="In allegato: $FILE_NAME" --attach="$FILE_NAME@application/excel"
/usr/bin/smtp-cli --verbose --server=smtp.boxlinesrl.com --from=info@boxlinesrl.com --to=magazzino@boxlinesrl.com --to=micheleperusi@boxlinesrl.com  --to=enniogia@gmail.com --to=lucaguerrini74@gmail.com --user=info@boxlinesrl.com --pass=box@info --missing-modules-ok --subject="FILE BOXLINE: $ZIP_NAME" --body-plain="In allegato: $ZIP_NAME" --attach="$ZIP_NAME@application/zip"

exit 0

