#!/bin/bash

# scrive i files sorgente per new dlg from another (correct) one
# usage convert_to_ui.sh <old name> <new name>
#
template_directory=../template

old_name=$1
new_name=$2

# old name uppercase
o_n_l=${old_name,,}

# old name lowercase
o_n_u=${old_name^^}

# old name capitalized
o_n_c=`echo "$old_name" | sed -r 's/(^|_)([a-z])/\U\2/g'`

# old name with  minus
o_n_m=`echo "$old_name" | sed -r 's/_/-/g'`

# new name uppercase
n_n_l=${new_name,,}

# new name lowercase
n_n_u=${new_name^^}

# new name capitalized
n_n_c=`echo "$new_name" | sed -r 's/(^|_)([a-z])/\U\2/g'`

# new name with  minus
n_n_m=`echo "$new_name" | sed -r 's/_/-/g'`


echo 'old file name        ' $file_old
echo 'old name uppercase   ' $o_n_u
echo 'old name lowercase   ' $o_n_l
echo 'old name capitalized ' $o_n_c
echo 'old name with minus  ' $o_n_m

echo ''

echo 'new file name        ' $file_new
echo 'new name uppercase   ' $n_n_u
echo 'new name lowercase   ' $n_n_l
echo 'new name capitalized ' $n_n_c
echo 'new name with minus  ' $n_n_m

cat $template_directory/$old_name.c | sed 's/'$o_n_u'/'$n_n_u'/g'  | sed 's/'$o_n_c'/'$n_n_c'/g' | sed 's/'$o_n_l'/'$n_n_l'/g' | sed 's/'$o_n_m'/'$n_n_m'/g' > $new_name.c
cat $template_directory/$old_name.h | sed 's/'$o_n_u'/'$n_n_u'/g'  | sed 's/'$o_n_c'/'$n_n_c'/g' | sed 's/'$o_n_l'/'$n_n_l'/g' | sed 's/'$o_n_m'/'$n_n_m'/g' > $new_name.h
