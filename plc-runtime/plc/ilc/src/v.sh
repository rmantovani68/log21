#!/bin/bash

versione=`cat VERSIONE`; \
echo -en 'char Versione[] = "@(#) '"$versione" > ver.c; \
echo -en '\\t\\t[' >> ver.c; \
echo -en "`date +"%d/%m/%Y %H:%M:%S"`" >> ver.c; \
echo -en ']\\tlib: ??? ";\n' >> ver.c

