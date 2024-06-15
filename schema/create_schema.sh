#!/bin/bash

#
# in ordine di dipendenza
#

for i in \
    ep_cedole.sql \
    ep_isole.sql \
    ep_imballi.sql \
    ep_pulsanti.sql \
    ep_operatori.sql \
    ep_prodotti.sql \
    ep_ordini.sql \
    ep_ordini_isola.sql \
    ep_colli.sql \
    ep_righe.sql \
    ep_pulsanti_settore.sql \
    ep_righe_settore.sql \
    ep_imballi_settore.sql \
    ep_cedole_settore.sql \
    ep_operatori_settore.sql \
    ep_settori.sql \
    ep_displays.sql \
    ep_ubicazioni.sql \
    ep_linea.sql \
    ; \
do echo $i; psql mr < $i; done
