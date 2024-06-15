#!/bin/bash

mkdir -p /usr/local/bin
mkdir -p /usr/local/cni
mkdir -p /usr/local/include
mkdir -p /usr/local/lib
mkdir -p /usr/lib/tcl8.0
mkdir -p /usr/lib/tk8.0

cd /usr/local ; tar xf /home/roberto/prj/edit_plc/usr_local_bin.tgz
cd /usr/local ; tar xf /home/roberto/prj/edit_plc/usr_local_cni.tgz
cd /usr/local ; tar xf /home/roberto/prj/edit_plc/usr_local_include.tgz
cd /usr/local ; tar xf /home/roberto/prj/edit_plc/usr_local_lib.tgz

cd /home/roberto/prj/edit_plc

cp wishDeb/wishDeb.tcltk8.0.ok /usr/local/bin/wishDeb
cp wishDeb/libtcl8.0.so /usr/local/lib
cp wishDeb/libtixsam4.1.8.0.so /usr/local/lib
cp wishDeb/libtk8.0.so /usr/local/lib
cp wishDeb/tcl8.0/* /usr/lib/tcl8.0
cp wishDeb/tk8.0/* /usr/lib/tk8.0

chmod a+rwx /usr/lib/tcl8.0
chmod a+rwx /usr/lib/tk8.0

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
