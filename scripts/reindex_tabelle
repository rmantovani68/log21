#!/bin/bash

#
# Ri-Creazione indici
#

#
# default values
#
DATABASE_NAME=`cat ../doc/BASE_NAME`
DATABASE_HOST=$HOSTNAME
REINDEX=reindex
OUTPUT_FILE=reindex.out

echo 'Creazione Indici'
echo -n 'Nome Database ['$DATABASE_NAME'] ? : '
read DB_NAME
echo -n 'Nome Host ['$DATABASE_HOST'] ? : '
read DB_HOST

if [ a"$DB_NAME" != a''  ]
then
DATABASE_NAME=$DB_NAME
fi

if [ a"$DB_HOST" != a''  ]
then
DATABASE_HOST=$DB_HOST
fi


# tabelle operative
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_ord.cfg     -t ric_ord     > $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_art.cfg     -t ric_art    >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_note.cfg    -t ric_note   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_quad.cfg    -t ric_quad   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ubicazioni.cfg  -t ubicazioni >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/catalogo.cfg    -t catalogo   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/col_prod.cfg    -t col_prod   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/rig_prod.cfg    -t rig_prod   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/quad_prod.cfg   -t rig_prod   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/imballi.cfg     -t imballi    >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/eventi.cfg      -t eventi     >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/operatori.cfg   -t operatori  >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/mov_ubi.cfg     -t mov_ubi    >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/settori.cfg     -t settori    >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/displays.cfg    -t displays   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/province.cfg    -t province   >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/regioni.cfg     -t regioni    >> $OUTPUT_FILE 2>&1 ; echo -n .

#storico
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_ord.cfg     -t ric_ord_stor  >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_art.cfg     -t ric_art_stor  >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ric_note.cfg    -t ric_note_stor >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/col_prod.cfg    -t col_prod_stor >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/rig_prod.cfg    -t rig_prod_stor >> $OUTPUT_FILE 2>&1 ; echo -n .


# Tabelle per gestione instradamento
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttcs.cfg -t ttcs >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttfc.cfg -t ttfc >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttfp.cfg -t ttfp >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttgc.cfg -t ttgc >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttls.cfg -t ttls >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttlv.cfg -t ttlv >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttrc.cfg -t ttrc >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttts.cfg -t ttts >> $OUTPUT_FILE 2>&1 ; echo -n .
$REINDEX -h $DATABASE_HOST -d $DATABASE_NAME -c ../data/ttve.cfg -t ttve >> $OUTPUT_FILE 2>&1 ; echo -n .

echo
echo 'Terminato'
