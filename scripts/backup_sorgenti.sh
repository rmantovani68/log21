#---------------------------------------
# backup completo sorgenti e documentazione
#---------------------------------------
PROJECT_NAME=`cat ../doc/BASE_NAME`
PROJECT_VERSION=`cat ../doc/BASE_VERSION`

DATE=`date +%d-%m-%y`

echo 'Project Sources Backup'
echo -n 'Nome Progetto ['$PROJECT_NAME'] ? : '
read PRJ_NAME
echo -n 'Versione ['$PROJECT_VERSION'] ? : '
read PRJ_VERSION

if [ a"$PRJ_NAME"a != a''a  ]
then
PROJECT_NAME=$PRJ_NAME
fi

if [ a"$PRJ_VERSION"a != a''a  ]
then
PROJECT_VERSION=$PRJ_VERSION
echo $PROJECT_VERSION > ../doc/BASE_VERSION
fi

# mi sposto sulla root del progetto
cd ..

cp exe/ael.cfg exe/ael.cfg.$DATE
cp exe/cni.cfg exe/cni.cfg.$DATE
cp exe/edit.cfg exe/edit.cfg.$DATE
cp data/printlabel.cfg data/printlabel.cfg.$DATE

if [ ! -d backup ]
then
	echo "Creating [backup] directory"
	mkdir backup
fi
if [ ! -d export ]
then
	echo "Creating [export] directory"
	mkdir export
fi
if [ ! -d stampe ]
then
	echo "Creating [stampe] directory"
	mkdir stampe
fi
if [ ! -d trace ]
then
	echo "Creating [trace] directory"
	mkdir trace
fi

tar zcf sorg.tgz \
exe/ael.cfg.$DATE \
exe/cni.cfg.$DATE \
exe/edit.cfg.$DATE \
[Mm]akefile */[Mm]akefile */*/[Mm]akefile \
*/*.[ch] */*/*.[ch] \
scripts/* \
sql/* \
tools/* \
data/*.cfg* \
*/VERSION \
.*/*.json \
*/*.sh \
*/*.css */*/*.css \
*/*.ui  */*/*.ui  \
*/*.xml */*/*.xml \
*/*.svg */*/*.svg \
*/*.glad* \
*/*.php \
*/*.png \
*/*.gif \
*/*.xpm \
exe/delete_ipc \
data/*.rc  \
doc/* \
*/*.mak \
pixmaps/* \
plc/*


rm exe/cni.cfg.$DATE
rm exe/ael.cfg.$DATE
rm exe/edit.cfg.$DATE

cp sorg.tgz backup/sorg.tgz
mv sorg.tgz backup/$PROJECT_NAME-$PROJECT_VERSION.tgz

echo 'Creato File di backup sorgenti : backup/'$PROJECT_NAME'-'$PROJECT_VERSION'.tgz'
ls -l  backup/$PROJECT_NAME-$PROJECT_VERSION.tgz

