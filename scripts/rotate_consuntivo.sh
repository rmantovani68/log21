#!/bin/bash

anno=`date +%Y`
data=`date +%d-%m-%y`
ora=`date +%H:%M:%S`

echo 'DATA :' $data
echo 'ORA  :' $ora
for i in /u/prj/mondadori/export/conscni.txt.old;do echo $i;mv $i $i-$data;gzip $i-$data;mv $i-$data.gz /u/prj/storico/export/$anno/consuntivi;done
