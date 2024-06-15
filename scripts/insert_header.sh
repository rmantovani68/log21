for i in *.c  `find ./ -maxdepth 1 -mindepth 1 -name  \*.c -a -not -name resources.c`
do
    echo $i
    echo -en '/*\n* @file '`basename $i`'\n*\n*/\n' > tmp; cat $i >> tmp
done
