#!/bin/bash
DB_HOST=database
DB_NAME=mondadori

echo ESTRAZIONE DATI DA CEDOLA $1

echo "drop table ric_ord_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "drop table ric_art_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "drop table col_prod_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "drop table rig_prod_cedola_"$1";"|psql -h $DB_HOST $DB_NAME

echo "create table ric_ord_cedola_"$1" as select * from ric_ord_stor where ordprog in (select ordprog from ric_ord_stor where ronmced='"$1"');"|psql -h $DB_HOST $DB_NAME
echo "create table ric_art_cedola_"$1" as select * from ric_art_stor where ordprog in (select ordprog from ric_ord_cedola_"$1" where ronmced='"$1"');"|psql -h $DB_HOST $DB_NAME
echo "create table col_prod_cedola_"$1" as select * from col_prod_stor where ordprog in (select ordprog from ric_ord_cedola_"$1" where ronmced='"$1"');"|psql -h $DB_HOST $DB_NAME
echo "create table rig_prod_cedola_"$1" as select * from rig_prod_stor where ordprog in (select ordprog from ric_ord_cedola_"$1" where ronmced='"$1"');"|psql -h $DB_HOST $DB_NAME

echo "insert into ric_ord select * from  ric_ord_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "insert into ric_art select * from  ric_art_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "insert into col_prod select * from  col_prod_cedola_"$1";"|psql -h $DB_HOST $DB_NAME
echo "insert into rig_prod select * from  rig_prod_cedola_"$1";"|psql -h $DB_HOST $DB_NAME

