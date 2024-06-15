#!/bin/bash

DOC_FOLDER=doc
BACKUP_FOLDER=backup

PROJECT_NAME=`cat $DOC_FOLDER/BASE_NAME`
PROJECT_VERSION=`cat $DOC_FOLDER/BASE_VERSION`
BUILD_NUMBER=`cat $DOC_FOLDER/BUILD_NUMBER`

DATE=`date +%d-%m-%y`

if [ -z "${BUILD_NUMBER}" ]
then
    BUILD_NUMBER=1
else
    BUILD_NUMBER=$((BUILD_NUMBER+1))
fi

echo 'BUILD_NUMBER : '$BUILD_NUMBER
echo $BUILD_NUMBER > $DOC_FOLDER/BUILD_NUMBER

echo -n '['$PROJECT_NAME']-['$PROJECT_VERSION']'

#echo 'PSB'
#echo -n 'NP['$PROJECT_NAME'] ? : '
#read PRJ_NAME
#echo -n 'V['$PROJECT_VERSION'] ? : '
#read PRJ_VERSION
#
#if [ a"$PRJ_NAME"a != a''a  ]
#then
#PROJECT_NAME=$PRJ_NAME
#fi
#
#if [ a"$PRJ_VERSION"a != a''a  ]
#then
#PROJECT_VERSION=$PRJ_VERSION
#echo $PROJECT_VERSION > ../docs/BASE_VERSION
#fi

PROJECT_VERSION=$PROJECT_VERSION"."$BUILD_NUMBER
echo 'PROJECT_VERSION='$PROJECT_VERSION

if [ ! -d $BACKUP_FOLDER ]
then
	echo "Creating [$BACKUP_FOLDER] directory"
	mkdir -p $BACKUP_FOLDER
fi

mv exe/ael.cfg exe/ael.cfg.$DATE
mv exe/edit.cfg exe/edit.cfg.$DATE

rm -f tmp_file_list.txt
find ./ -wholename '*.[CHch]*'        -print0 >> tmp_file_list.txt
find ./ -wholename '*.cfg*'           -print0 >> tmp_file_list.txt
find ./ -wholename '*.svg'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.rb'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.build'          -print0 >> tmp_file_list.txt
find ./ -wholename '*.sh'             -print0 >> tmp_file_list.txt
find ./ -wholename '*/[Mm]akefile'    -print0 >> tmp_file_list.txt
find ./ -wholename '*.mak'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.pdf'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.py'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.dox*'           -print0 >> tmp_file_list.txt
find ./ -wholename '*.dxy'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.md'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.sh'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.bat'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.prj'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.txt'            -print0 >> tmp_file_list.txt
find ./ -wholename '*/config/*'       -print0 >> tmp_file_list.txt
find ./ -wholename '*/images/*'       -print0 >> tmp_file_list.txt
find ./ -wholename '*/doxygen/*'      -print0 >> tmp_file_list.txt
find ./ -wholename '*/scripts/*'      -print0 >> tmp_file_list.txt
find ./ -wholename '*/doc/*'          -print0 >> tmp_file_list.txt
find ./ -wholename '*.xml'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.yml'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.ui'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.ld'             -print0 >> tmp_file_list.txt
find ./ -wholename '*.iss'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.sln'            -print0 >> tmp_file_list.txt
find ./ -wholename '*.vcxproj'        -print0 >> tmp_file_list.txt
find ./ -wholename '*.filters'        -print0 >> tmp_file_list.txt
find ./ -wholename '*.csv'            -print0 >> tmp_file_list.txt
find ./ -wholename '*/config/[A-Z]*'  -print0 >> tmp_file_list.txt
find ./ -wholename '*/doc/[A-Z]*'     -print0 >> tmp_file_list.txt
find ./ -wholename '*/derivatives/*/doc/[A-Z]*'  -print0 >> tmp_file_list.txt

rm -f $BACKUP_FOLDER/$PROJECT_NAME-$PROJECT_VERSION.tgz


tar \
--exclude='backup/*' \
--exclude='kernel*/*' \
--exclude='resources.c' \
--exclude='*.a' \
--exclude='*.o' \
--exclude='*.hex' \
--exclude='*.s19' \
--exclude='*.axf' \
--exclude='code/*' \
--exclude='*.so*' \
--exclude='*.chm' \
--exclude='work/resources' \
--exclude='work/build' \
--exclude='doc/html' \
--exclude='doc/latex' \
--exclude='sysroot' \
--exclude='resources/mingw*' \
--exclude='golden' \
--exclude='tmp_file_list.txt' \
--exclude='src/tests/projects' \
--exclude='resources/examples' \
--exclude='log/*' \
--exclude='trace/*' \
--exclude='venv' \
--exclude='*/__pycache__' \
-zcf $BACKUP_FOLDER/$PROJECT_NAME-$PROJECT_VERSION.tgz  --files-from tmp_file_list.txt 2> /dev/null

rm -f tmp_file_list.txt

mv exe/ael.cfg.$DATE    exe/ael.cfg
mv exe/edit.cfg.$DATE   exe/edit.cfg

#gpg -c -o $BACKUP_FOLDER/$PROJECT_NAME-$PROJECT_VERSION.tgz tmp_src.tgz
#mv tmp_src.tgz $BACKUP_FOLDER//$PROJECT_NAME-$PROJECT_VERSION.tgz
#rm tmp_src.tgz

echo 'Creato File di backup sorgenti : '$BACKUP_FOLDER'/'$PROJECT_NAME'-'$PROJECT_VERSION'.tgz'
ls -l  $BACKUP_FOLDER/$PROJECT_NAME-$PROJECT_VERSION.tgz
dir_name=$PROJECT_NAME-$PROJECT_VERSION
cd backup &&  mkdir $dir_name && cd $dir_name && split ../$PROJECT_NAME-$PROJECT_VERSION.tgz -b 10000000 && for i in *; do echo $i; done


