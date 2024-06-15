#!/bin/bash
DBHOST=localhost
DBNAME=mondadori
echo 'drop table storico_scarti_backup;' | psql -h $DBHOST $DBNAME 
echo 'create table storico_scarti_backup as select * from storico_scarti;' | psql -h $DBHOST $DBNAME 
echo 'truncate table storico_scarti;' | psql -h $DBHOST $DBNAME 


