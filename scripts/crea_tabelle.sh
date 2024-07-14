#!/bin/bash

#
# Creazione tabelle / indici / sequenze / funzioni
#

#
# default values
#
PGDB=`cat ../doc/DB_NAME`
PGHOST=`cat ../doc/DB_HOST`
DBF2SQL=../exe/dbf2sql

echo 'Creazione Tabelle' > crea_tabelle.out

echo 'Creazione Tabelle'

echo -n 'Nome Database ['$PGDB'] ? : '
read DBNAME
echo -n 'Nome Host ['$PGHOST'] ? : '
read DBHOST

if [ a"$DBNAME" != a''  ]
then
export PGDB=$DBNAME
fi

if [ a"$DBHOST" != a''  ]
then
export PGHOST=$DBHOST
fi

export PGUSER=postgres
export PGPASSWORD=3zin

echo "PGDB :" $PGDB
echo "PGHOST :" $PGHOST
echo "PGUSER :" $PGUSER
echo "PGPASSWORD :" $PGPASSWORD

# psql -U $PGUSER -h $PGHOST -d $PGDB -f install_plpgsql.sql >> crea_tabelle.out 2>&1 ; echo -n .

echo 'Creazione insert_update_triggers' >> crea_tabelle.out 2>&1 
psql -U $PGUSER -h $PGHOST -d $PGDB -f insert_update_triggers.sql >> crea_tabelle.out 2>&1 ; echo -n .

# Sequenze progressive 
echo 'CREATE SEQUENCE proglancio increment 1 minvalue 0 maxvalue 999999 start 1000;'                | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE sequence_anno increment 1 minvalue 2000 maxvalue 4000 start 2000;'            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE sequence_xab increment 1 minvalue 0 maxvalue 999999 start 1;'                 | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE sequence_distinta increment 1 minvalue 0 maxvalue 999999 start 1;'            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE sequence_cedola increment 1 minvalue 0 maxvalue 999999 start 1;'              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

# Sequenze progressive - carichi
echo 'CREATE SEQUENCE carichi_codice_udc_seq increment 1 minvalue 400000 maxvalue 700000 start 400001;' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE carichi_numero_distinta_seq increment 1 minvalue 1 maxvalue 9999999 start 1;'     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE SEQUENCE carichi_numero_viaggio_seq increment 1 minvalue 1 maxvalue 9999999 start 1;'      | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .


# Funzioni di rappresentazione codice ubicazione e prodotto
echo "DROP FUNCTION IF EXISTS  settore(bpchar);"            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION settore(bpchar) returns text as 'select substr(\$1,1, 2)' language 'sql';"     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  colonna(bpchar);"            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION colonna(bpchar) returns text as 'select substr(\$1,3, 2)' language 'sql';"     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  piano(bpchar);"              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION piano(bpchar)   returns text as 'select substr(\$1,5, 2)' language 'sql';"     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ubicazione(bpchar);"         | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION ubicazione(bpchar)          returns text as 'select substr(\$1,1,2)||''.''||substr(\$1,3,2)||''.''||substr(\$1,5,2)' language 'sql';" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ubicazione_ridotta(bpchar);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION ubicazione_ridotta(bpchar)  returns text as 'select substr(\$1,1,2)||''.''||substr(\$1,3,2)||''.''||substr(\$1,5,2)' language 'sql';" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ubicazione_rid(bpchar);"     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION ubicazione_rid(bpchar)      returns text as 'select substr(\$1,1,2)||''.''||substr(\$1,3,2)||''.''||substr(\$1,5,2)' language 'sql';" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  codprod_ridotto(bpchar);"    | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ;echo "CREATE FUNCTION codprod_ridotto(bpchar)     returns text as 'select substr(\$1,6,7)' language 'sql';" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

# Funzioni di gestione data/ora (using to_char())

echo "DROP FUNCTION IF EXISTS  data_YYYYMMDDHHMMSS(timestamp);"        | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_YYYYMMDDHHMMSS(timestamptz);"      | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_YYYYMMDD(timestamp);"              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_YYYYMMDD(timestamptz);"            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_YYYYMMDD(text);"                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_DDMMYY(timestamp);"                | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_DDMMYY(timestamptz);"              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora(timestamp);"                        | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora(timestamptz);"                      | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data(timestamp);"                       | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data(timestamptz);"                     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_ora(timestamp);"                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_ora(timestamptz);"                 | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_HH(timestamp);"                     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_HH(timestamptz);"                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_MM(timestamp);"                     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_MM(timestamptz);"                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_SS(timestamp);"                     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  ora_SS(timestamptz);"                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_DDMMYYYYcolonHHMMSS(timestamp);"   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  data_DDMMYYYYcolonHHMMSS(timestamptz);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  str2int(varchar); "                        | psql -h $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "DROP FUNCTION IF EXISTS  timestamp_from_YYYYMMDDHHMMSS(text); "  | psql -h $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .


echo "CREATE FUNCTION data_YYYYMMDDHHMMSS(timestamp) returns text as 'select to_char(\$1,''YYYYMMDDHH24MISS'')' language 'sql';"                                                                                                                                | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_YYYYMMDDHHMMSS(timestamptz) returns text as 'select to_char(\$1,''YYYYMMDDHH24MISS'')' language 'sql';"                                                                                                                              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_YYYYMMDD(timestamp) returns text as 'select to_char(\$1,''YYYYMMDD'')' language 'sql';"                                                                                                                                              | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_YYYYMMDD(timestamptz) returns text as 'select to_char(\$1,''YYYYMMDD'')' language 'sql';"                                                                                                                                            | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_YYYYMMDD(text) returns text as 'select substr(\$1,7,2)||''/''||substr(\$1,5,2)||''/''||substr(\$1,3,2)' language 'sql';"                                                                                                             | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_DDMMYY(timestamp) returns text as 'select to_char(\$1,''DDMMYY'')' language 'sql';"                                                                                                                                                  | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_DDMMYY(timestamptz) returns text as 'select to_char(\$1,''DDMMYY'')' language 'sql';"                                                                                                                                                | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora(timestamp) returns text as 'select to_char(\$1,''HH24:MI:SS'')' language 'sql';"                                                                                                                                                      | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora(timestamptz) returns text as 'select to_char(\$1,''HH24:MI:SS'')' language 'sql';"                                                                                                                                                    | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data(timestamp) returns text as 'select to_char(\$1,''DD-MM-YYYY'')' language 'sql';"                                                                                                                                                     | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data(timestamptz) returns text as 'select to_char(\$1,''DD-MM-YYYY'')' language 'sql';"                                                                                                                                                   | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_ora(timestamp) returns text as 'select data(\$1)||'' ''||ora(\$1)' language 'sql';"                                                                                                                                                  | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_ora(timestamptz) returns text as 'select data(\$1)||'' ''||ora(\$1)' language 'sql';"                                                                                                                                                | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_HH(timestamp) returns text as 'select to_char(\$1,''HH24'')' language 'sql';"                                                                                                                                                         | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_HH(timestamptz) returns text as 'select to_char(\$1,''HH24'')' language 'sql';"                                                                                                                                                       | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_MM(timestamp) returns text as 'select to_char(\$1,''MI'')' language 'sql';"                                                                                                                                                           | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_MM(timestamptz) returns text as 'select to_char(\$1,''MI'')' language 'sql';"                                                                                                                                                         | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_SS(timestamp) returns text as 'select to_char(\$1,''SS'')' language 'sql';"                                                                                                                                                           | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_SS(timestamptz) returns text as 'select to_char(\$1,''SS'')' language 'sql';"                                                                                                                                                         | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_DDMMYYYYcolonHHMMSS(timestamp) returns text as 'select to_char(\$1,''DDMMYYYY:HH24MISS'')' language 'sql';"                                                                                                                          | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE FUNCTION data_DDMMYYYYcolonHHMMSS(timestamptz) returns text as 'select to_char(\$1,''DDMMYYYY:HH24MISS'')' language 'sql';"                                                                                                                        | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo " \
create or replace function str2int(val varchar) returns bigint immutable as \$\$ \
begin \
      val=substring(val from '[0-9]*'); \
      if length(val) between 1 and 19 then return val::bigint; end if; \
      return 0; \
end; \
\$\$ language plpgsql; \
" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo "CREATE FUNCTION timestamp_from_YYYYMMDDHHMMSS(text) returns timestamp as 'select (to_char(greatest(1,str2int(substr(\$1,1,4))),''FM0999'')||''-''||to_char(greatest(1,str2int(substr(\$1,5,2))),''FM09'')||''-''||to_char(greatest(1,str2int(substr(\$1,7,2))),''FM09'')||'' ''||to_char(str2int(substr(\$1,9,2)),''FM09'')||'':''||to_char(str2int(substr(\$1,11,2)),''FM09'')||'':''||to_char(str2int(substr(\$1,13,2)),''FM09''))::timestamp' language 'sql';" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE or REPLACE FUNCTION anno(timestamp) returns text as 'select to_char(\$1,''YYYY'')' language 'sql';"                                                                                                                                                           | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "CREATE or REPLACE FUNCTION anno(timestamptz) returns text as 'select to_char(\$1,''YYYY'')' language 'sql';"                                                                                                                                                         | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

# tabelle operative - ripulire
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/cedole.cfg      -t cedole     >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_ord.cfg     -t ric_ord    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_art.cfg     -t ric_art    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_note.cfg    -t ric_note   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_quad.cfg    -t ric_quad   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/catalogo.cfg    -t catalogo   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/col_prod.cfg    -t col_prod   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/rig_prod.cfg    -t rig_prod   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/quad_prod.cfg   -t quad_prod  >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/eventi.cfg      -t eventi     >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/operatori.cfg   -t operatori  >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/mov_ubi.cfg     -t mov_ubi    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ubicazioni.cfg  -t ubicazioni >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/imballi.cfg     -t imballi    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/settori.cfg     -t settori    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/displays.cfg    -t displays   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/province.cfg    -t province   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/regioni.cfg     -t regioni    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/nota_linee.cfg  -t nota_linee >> crea_tabelle.out 2>&1 ; echo -n .

# import 
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/import_ric_ord.cfg     -t import_ric_ord    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/import_ric_art.cfg     -t import_ric_art    >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/import_catalogo.cfg    -t import_catalogo   >> crea_tabelle.out 2>&1 ; echo -n .

# export 
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/flusso_cedola.cfg      -t flusso_cedola     >> crea_tabelle.out 2>&1 ; echo -n .

# storico - ripulire
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/cedole.cfg      -t cedole_stor   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_ord.cfg     -t ric_ord_stor  >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_art.cfg     -t ric_art_stor  >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ric_note.cfg    -t ric_note_stor >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/col_prod.cfg    -t col_prod_stor >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/rig_prod.cfg    -t rig_prod_stor >> crea_tabelle.out 2>&1 ; echo -n .


# Tabelle per gestione instradamento - eliminare
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttcs.cfg -t ttcs >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttfc.cfg -t ttfc >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttfp.cfg -t ttfp >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttgc.cfg -t ttgc >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttls.cfg -t ttls >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttlv.cfg -t ttlv >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttrc.cfg -t ttrc >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttts.cfg -t ttts >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttve.cfg -t ttve >> crea_tabelle.out 2>&1 ; echo -n .

# Tabella descrizione spedizione per cedola - eliminare
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/ttds.cfg -t ttds >> crea_tabelle.out 2>&1 ; echo -n .

# gestione colli scartati
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/colli_scartati.cfg -t colli_scartati >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/colli_fuori_linea.cfg -t colli_fuori_linea >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/scarti.cfg -t scarti >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/scarti_flags.cfg -t scarti_flags >> crea_tabelle.out 2>&1 ; echo -n .

# tabelle - carichi
# tabelle import/export
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_import_carichi.cfg      -t  carichi_import_carichi        >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_export_rimanenze.cfg    -t  carichi_export_rimanenze      >> crea_tabelle.out 2>&1 ; echo -n .

# tabelle operative
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_giacenze.cfg            -t  carichi_giacenze              >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_udc.cfg                 -t  carichi_udc                   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_prodotti_udc.cfg        -t  carichi_prodotti_udc          >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_movimentazioni.cfg      -t  carichi_movimentazioni        >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_distinte.cfg            -t  carichi_distinte              >> crea_tabelle.out 2>&1 ; echo -n .

# storico
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_giacenze.cfg            -t  carichi_giacenze_stor         >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_udc.cfg                 -t  carichi_udc_stor              >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_prodotti_udc.cfg        -t  carichi_prodotti_udc_stor     >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_movimentazioni.cfg      -t  carichi_movimentazioni_stor   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_distinte.cfg            -t  carichi_distinte_stor         >> crea_tabelle.out 2>&1 ; echo -n .

$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_operatori.cfg           -t  carichi_operatori             >> crea_tabelle.out 2>&1 ; echo -n .

# verificare 
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_spedizioni_ricevute.cfg -t  carichi_spedizioni_ricevute   >> crea_tabelle.out 2>&1 ; echo -n .
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/carichi_distinte_export.cfg     -t  carichi_distinte_export       >> crea_tabelle.out 2>&1 ; echo -n .



# Tabella delle letture (storico)
echo 'CREATE SEQUENCE sequence_scarti increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_scarti (
	key       INTEGER DEFAULT NEXTVAL('sequence_scarti'),
	sscdpro   TEXT DEFAULT 'CODICE',
	sscdflg   TEXT,
	ssprgid   INTEGER,
	sstmread  TIMESTAMP
);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

# caricamento funzioni sql per controllo cedola
psql -U $PGUSER -h $PGHOST -d $PGDB -f ../scripts/mondadori.sql >> crea_tabelle.out 2>&1 ; echo -n .

# Tabella linee di spedizione (storico)
echo 'CREATE SEQUENCE sequence_linee_spedizione increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_linee_spedizione (
	key_linea   INTEGER,
	tminit      TIMESTAMP,
	tmclose     TIMESTAMP,
	lnnmced     INTEGER,
	lntpspe     VARCHAR(20) DEFAULT 'TPSPE',
	lncdlin     VARCHAR(20) DEFAULT 'CDLIN',
	stato       VARCHAR(1) DEFAULT ' '
);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo 'CREATE UNIQUE INDEX storico_linee_spedizione_key ON storico_linee_spedizione (key_linea);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .


# Tabella bancali di spedizione (storico)
echo 'CREATE SEQUENCE sequence_bancali increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_bancali (
	key_linea   INTEGER,
	key_bancale INTEGER,
	tminit      TIMESTAMP,
	tmclose     TIMESTAMP,
	bnnmced     INTEGER,
	bnnmbnc     INTEGER,
	bntpspe     VARCHAR(20) DEFAULT 'TPSPE',
	bncdlin     VARCHAR(20) DEFAULT 'CDLIN',
	stato       VARCHAR(1) DEFAULT ' '
);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo 'CREATE INDEX storico_bancali_key ON storico_bancali (key_linea);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE UNIQUE INDEX storico_bancali_key_bancale ON storico_bancali (key_bancale);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

# Tabella letture colli su bancali (storico)
echo 'CREATE SEQUENCE sequence_colli_bancale increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_colli_bancale (
	key_collo    INTEGER,
	key_bancale  INTEGER,
	key_linea    INTEGER,
	ordprog   VARCHAR(20) DEFAULT 'CODICE',
	cpnmced   INTEGER,
	cptpspe   VARCHAR(20) DEFAULT 'TPSPE',
	cpcdlin   VARCHAR(20) DEFAULT 'CDLIN',
	cpnmcol   INTEGER,
	cppsrea   INTEGER,
	cppspre   INTEGER,
	tmread    TIMESTAMP
);" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo 'CREATE UNIQUE INDEX storico_colli_bancale_key ON storico_colli_bancale (ordprog,cpnmcol);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE UNIQUE INDEX storico_colli_bancale_key_collo ON storico_colli_bancale (key_collo);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE INDEX storico_colli_bancale_key_bancale ON storico_colli_bancale (key_bancale);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .
echo 'CREATE INDEX storico_colli_bancale_key_linea ON storico_colli_bancale (key_linea);' | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>&1 ; echo -n .

echo "insert into carichi_operatori (opidope,oppwope) values ('00','00');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('01','01');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('02','02');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('03','03');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('04','04');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('05','05');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('06','06');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('07','07');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('08','08');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('09','09');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('10','10');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('11','11');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('12','12');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('13','13');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('14','14');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('15','15');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('16','16');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('17','17');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('18','18');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('19','19');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('20','20');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('21','21');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('22','22');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('23','23');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('24','24');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('25','25');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('26','26');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('27','27');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('28','28');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('29','29');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('30','30');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('31','31');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('32','32');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('33','33');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('34','34');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('35','35');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('36','36');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('37','37');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('38','38');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('39','39');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('40','40');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('41','41');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('42','42');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('43','43');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('44','44');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('45','45');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('46','46');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('47','47');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('48','48');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('49','49');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('50','50');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('51','51');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('52','52');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('53','53');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('54','54');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('55','55');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('56','56');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('57','57');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('58','58');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('59','59');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('60','60');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('61','61');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('62','62');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('63','63');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('64','64');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('65','65');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('66','66');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('67','67');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('68','68');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('69','69');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('70','70');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('71','71');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('72','72');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('73','73');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('74','74');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('75','75');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('76','76');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('77','77');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('78','78');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('79','79');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('80','80');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('81','81');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('82','82');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('83','83');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('84','84');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('85','85');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('86','86');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('87','87');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('88','88');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('89','89');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('90','90');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('91','91');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('92','92');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('93','93');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('94','94');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('95','95');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('96','96');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('97','97');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('98','98');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into carichi_operatori (opidope,oppwope) values ('99','99');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .


# Tabella shm_linea
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/shm_linea.cfg     -t  shm_linea       >> crea_tabelle.out 2>&1 ; echo -n .
echo "insert into shm_linea values ('ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .

# Tabella shm_settori
$DBF2SQL -i -U $PGUSER -P $PGPASSWORD -h $PGHOST -d $PGDB -c ../data/shm_linea.cfg     -t  shm_linea       >> crea_tabelle.out 2>&1 ; echo -n .
echo "insert into shm_settori values ('1,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('2,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('3,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('4,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('5,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('6,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('7,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('8,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('9,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('10,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('11,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('12,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('13,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('14,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('15,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('16,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('17,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('18,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('19,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .
echo "insert into shm_settori values ('20,'ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR','ERR');" | psql -U $PGUSER -h $PGHOST $PGDB >> crea_tabelle.out 2>1 ; echo -n .

echo
echo 'Terminato' >> crea_tabelle.out 2>1 ; echo 'Terminato'
