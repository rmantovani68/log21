#!/bin/bash

# Applicazione di fne linea - controllo bancali di spedizione
# Creazione tabelle / indici / sequenze / funzioni
#
OUT_FILE=crea_tabelle_check_bancali.out

#
# default values
#
DATABASE_NAME=`cat ../doc/BASE_NAME`
DATABASE_HOST=$HOSTNAME

echo 'Creazione Tabelle'
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

DATE=`date +%d-%m-%y`
echo $DATE > $OUT_FILE

# Funzioni di rappresentazione codice ubicazione
echo "CREATE FUNCTION fila(bpchar) returns text as 'select substr(\$1,1,3)' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION montante(bpchar) returns text as 'select substr(\$1,4,3)' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION colonna(bpchar) returns text as 'select substr(\$1,7,3)' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION piano(bpchar) returns text as 'select substr(\$1,10,3)' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION ubicazione(bpchar) returns text as 'select substr(\$1,1,3)||\'.\'||substr(\$1,4,3)||\'.\'||substr(\$1,7,3)||\'.\'||substr(\$1,10,3)' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

# Funzioni di gestione data/ora
echo "CREATE FUNCTION data_YYYYMMDDHHMMSS(datetime) returns text as 'select lpad(date_part(\'year\',\$1),4,\'0\')||lpad(date_part(\'month\',\$1),2,\'0\')||lpad(date_part(\'day\',\$1),2,\'0\')||lpad(date_part(\'hour\',\$1),2,\'0\')||lpad(date_part(\'minute\',\$1),2,\'0\')||lpad(date_part(\'second\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION data_YYYYMMDD(datetime) returns text as 'select lpad(date_part(\'year\',\$1),4,\'0\')||lpad(date_part(\'month\',\$1),2,\'0\')||lpad(date_part(\'day\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION ora(datetime) returns text as 'select lpad(date_part(\'hour\',\$1),2,\'0\')||\':\'||lpad(date_part(\'minute\',\$1),2,\'0\')||\':\'||lpad(date_part(\'second\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION data(datetime) returns text as 'select lpad(date_part(\'day\',\$1),2,\'0\')||\'-\'||lpad(date_part(\'month\',\$1),2,\'0\')||\'-\'||lpad(date_part(\'year\',\$1),4,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_HH(datetime) returns text as 'select lpad(date_part(\'hour\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_MM(datetime) returns text as 'select lpad(date_part(\'minute\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "CREATE FUNCTION ora_SS(datetime) returns text as 'select lpad(date_part(\'second\',\$1),2,\'0\')' language 'sql';" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

# Tabella linee di spedizione (storico)
echo 'CREATE SEQUENCE sequence_linee_spedizione increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_linee_spedizione (
	key_linea   INTEGER,
	tminit      DATETIME,
	tmclose     DATETIME,
	lntpspe     VARCHAR(20) DEFAULT 'TPSPE',
	lncdlin     VARCHAR(20) DEFAULT 'CDLIN',
	stato       VARCHAR(1) DEFAULT ' '
);" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

echo 'CREATE UNIQUE INDEX storico_linee_spedizione_key ON storico_linee_spedizione (key_linea);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .


# Tabella bancali di spedizione (storico)
echo 'CREATE SEQUENCE sequence_bancali increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_bancali (
	key_linea   INTEGER,
	key_bancale INTEGER,
	tminit      DATETIME,
	tmclose     DATETIME,
	bntpspe     VARCHAR(20) DEFAULT 'TPSPE',
	bncdlin     VARCHAR(20) DEFAULT 'CDLIN',
	stato       VARCHAR(1) DEFAULT ' '
);" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

echo 'CREATE INDEX storico_bancali_key ON storico_bancali (key_linea);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo 'CREATE UNIQUE INDEX storico_bancali_key_bancale ON storico_bancali (key_bancale);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

# Tabella letture colli su bancali (storico)
echo 'CREATE SEQUENCE sequence_colli_bancale increment 1 minvalue 1 maxvalue 99999999 start 1;' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo "\
CREATE TABLE storico_colli_bancale (
	key_collo    INTEGER,
	key_bancale  INTEGER,
	key_linea    INTEGER,
	ordprog   VARCHAR(20) DEFAULT 'CODICE',
	cptpspe   VARCHAR(20) DEFAULT 'TPSPE',
	cpcdlin   VARCHAR(20) DEFAULT 'CDLIN',
	cpnmcol   INTEGER,
	cppsrea   INTEGER,
	cppspre   INTEGER,
	tmread    DATETIME
);" | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

echo 'CREATE UNIQUE INDEX storico_colli_bancale_key ON storico_colli_bancale (ordprog,cpnmcol);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo 'CREATE UNIQUE INDEX storico_colli_bancale_key_collo ON storico_colli_bancale (key_collo);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo 'CREATE INDEX storico_colli_bancale_key_bancale ON storico_colli_bancale (key_bancale);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .
echo 'CREATE INDEX storico_colli_bancale_key_linea ON storico_colli_bancale (key_linea);' | psql -h $DATABASE_HOST $DATABASE_NAME >> $OUT_FILE 2>&1 ; echo -n .

echo
echo 'Terminato'
