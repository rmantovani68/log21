#!/bin/bash

anno=`date +%Y`
data=`date +%d-%m-%y`
ora=`date +%H:%M:%S`

echo 'DATA :' $data
echo 'ORA  :' $ora
for i in /u/prj/mondadori/export/??cni001.txt;do echo $i;mv $i $i-$data;gzip $i-$data;mv $i-$data.gz /u/prj/storico/export/$anno/cedole;done
