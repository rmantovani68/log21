rm -f tabelle_produzione.dump
echo   catalogo  ;pg_dump -h database mr -O -x  -t  catalogo   >> tabelle_produzione.dump
echo   imballi   ;pg_dump -h database mr -O -x  -t  imballi    >> tabelle_produzione.dump
echo   ric_ord   ;pg_dump -h database mr -O -x  -t  ric_ord    >> tabelle_produzione.dump
echo   ric_art   ;pg_dump -h database mr -O -x  -t  ric_art    >> tabelle_produzione.dump
echo   col_prod  ;pg_dump -h database mr -O -x  -t  col_prod   >> tabelle_produzione.dump
echo   rig_prod  ;pg_dump -h database mr -O -x  -t  rig_prod   >> tabelle_produzione.dump
echo   ubicazioni;pg_dump -h database mr -O -x  -t  ubicazioni >> tabelle_produzione.dump

gzip tabelle_produzione.dump
#sz tabelle_produzione.sql.gz
