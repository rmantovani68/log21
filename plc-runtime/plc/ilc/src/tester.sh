#!/bin/bash

exec \
ilc  -header ../runtm/h/scambio ../runtm/h/illib.ilc -code ../runtm/h/scambdin\
 -os test/scambdin.s  test/sfiga.plc -os test/sfiga.plc.s test/sfiga.msd\
 -os test/sfiga.msd.s # -debug 255

