#!/bin/bash

##
# restore del database picking MOMO
##

#
# default values
#
DATABASE_NAME=`cat ../doc/BASE_NAME`
DATABASE_HOST=$HOSTNAME

echo 'Restore Database'
echo -n 'Nome Database ['$DATABASE_NAME'] ? : '
read DB_NAME
echo -n 'Nome Host ['$DATABASE_HOST'] ? : '
read DB_HOST

DATABASE_BACKUP=../backup/$DATABASE_NAME.dump.gz

echo -n 'Nome File di Backup ['$DATABASE_BACKUP'] ? : '
read DB_BACKUP

if [ a"$DB_NAME" != a''  ]
then
DATABASE_NAME=$DB_NAME
fi

if [ a"$DB_HOST" != a''  ]
then
DATABASE_HOST=$DB_HOST
fi

if [ a"$DB_BACKUP" != a''  ]
then
DATABASE_BACKUP=$DB_BACKUP
fi

gunzip -c ../backup/$DATABASE_BACKUP | psql -h $DATABASE_HOST $DATABASE_NAME
