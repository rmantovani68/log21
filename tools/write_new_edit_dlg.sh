#!/bin/bash

# scrive i files sorgente per new dlg from another (correct) one
# usage convert_to_ui.sh <old file> <new file> <old name> <new name>
#

file_old=$1
file_new=$2
old_name=$3
new_name=$4


old_name_lower=${old_name,,}
old_name_upper=${old_name^^}
old_name_capitalized=${old_name^}

new_name_lower=${new_name,,}
new_name_upper=${new_name^^}
new_name_capitalized=${new_name^}

cat $file_old.c | sed 's/'$old_name_upper'/'$new_name_upper'/g'  | sed 's/'$old_name_capitalized'/'$new_name_capitalized'/g' | sed 's/'$old_name_lower'/'$new_name_lower'/g' > $file_new.c
cat $file_old.h | sed 's/'$old_name_upper'/'$new_name_upper'/g'  | sed 's/'$old_name_capitalized'/'$new_name_capitalized'/g' | sed 's/'$old_name_lower'/'$new_name_lower'/g' > $file_new.h
