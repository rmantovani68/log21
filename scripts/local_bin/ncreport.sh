#!/bin/bash

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/glibc-2.14/lib:/usr/local/Trolltech/Qt-4.8.5/lib/ 
export LD_LIBRARY_PATH
/usr/local/bin/NCReport $*

