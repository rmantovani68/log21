rm -f tabelle_produzione.dump
echo   catalogo  ;pg_dump --column-inserts -O -x  -h database -t  catalogo   mr >> tabelle_produzione.dump
echo   displays  ;pg_dump --column-inserts -O -x  -h database -t  displays   mr >> tabelle_produzione.dump
echo   eventi    ;pg_dump --column-inserts -O -x  -h database -t  eventi     mr >> tabelle_produzione.dump
echo   operatori ;pg_dump --column-inserts -O -x  -h database -t  operatori  mr >> tabelle_produzione.dump
echo   province  ;pg_dump --column-inserts -O -x  -h database -t  province   mr >> tabelle_produzione.dump
echo   quad_prod ;pg_dump --column-inserts -O -x  -h database -t  quad_prod  mr >> tabelle_produzione.dump
echo   regioni   ;pg_dump --column-inserts -O -x  -h database -t  regioni    mr >> tabelle_produzione.dump
echo   ric_art   ;pg_dump --column-inserts -O -x  -h database -t  ric_art    mr >> tabelle_produzione.dump
echo   ric_note  ;pg_dump --column-inserts -O -x  -h database -t  ric_note   mr >> tabelle_produzione.dump
echo   imballi   ;pg_dump --column-inserts -O -x  -h database -t  imballi    mr >> tabelle_produzione.dump
echo   ric_ord   ;pg_dump --column-inserts -O -x  -h database -t  ric_ord    mr >> tabelle_produzione.dump
echo   ric_quad  ;pg_dump --column-inserts -O -x  -h database -t  ric_quad   mr >> tabelle_produzione.dump
echo   col_prod  ;pg_dump --column-inserts -O -x  -h database -t  col_prod   mr >> tabelle_produzione.dump
echo   rig_prod  ;pg_dump --column-inserts -O -x  -h database -t  rig_prod   mr >> tabelle_produzione.dump
echo   settori   ;pg_dump --column-inserts -O -x  -h database -t  settori    mr >> tabelle_produzione.dump
echo   ttcs      ;pg_dump --column-inserts -O -x  -h database -t  ttcs       mr >> tabelle_produzione.dump
echo   ttds      ;pg_dump --column-inserts -O -x  -h database -t  ttds       mr >> tabelle_produzione.dump
echo   ttfc      ;pg_dump --column-inserts -O -x  -h database -t  ttfc       mr >> tabelle_produzione.dump
echo   ttfp      ;pg_dump --column-inserts -O -x  -h database -t  ttfp       mr >> tabelle_produzione.dump
echo   ttgc      ;pg_dump --column-inserts -O -x  -h database -t  ttgc       mr >> tabelle_produzione.dump
echo   ttls      ;pg_dump --column-inserts -O -x  -h database -t  ttls       mr >> tabelle_produzione.dump
echo   ttlv      ;pg_dump --column-inserts -O -x  -h database -t  ttlv       mr >> tabelle_produzione.dump
echo   ttrc      ;pg_dump --column-inserts -O -x  -h database -t  ttrc       mr >> tabelle_produzione.dump
echo   ttts      ;pg_dump --column-inserts -O -x  -h database -t  ttts       mr >> tabelle_produzione.dump
echo   ttve      ;pg_dump --column-inserts -O -x  -h database -t  ttve       mr >> tabelle_produzione.dump
echo   ubicazioni;pg_dump --column-inserts -O -x  -h database -t  ubicazioni mr >> tabelle_produzione.dump


# carichi
echo   carichi_catalogo_stor            ; pg_dump --column-inserts -O -x  -h database -t  carichi_catalogo_stor             mr >> tabelle_produzione.dump
echo   carichi_export_rimanenze_backup  ; pg_dump --column-inserts -O -x  -h database -t  carichi_export_rimanenze_backup   mr >> tabelle_produzione.dump
echo   carichi_lista_movimentazioni     ; pg_dump --column-inserts -O -x  -h database -t  carichi_lista_movimentazioni      mr >> tabelle_produzione.dump
echo   carichi_prodotti_udc             ; pg_dump --column-inserts -O -x  -h database -t  carichi_prodotti_udc              mr >> tabelle_produzione.dump
echo   carichi_udc_stor                 ; pg_dump --column-inserts -O -x  -h database -t  carichi_udc_stor                  mr >> tabelle_produzione.dump
echo   carichi_distinte                 ; pg_dump --column-inserts -O -x  -h database -t  carichi_distinte                  mr >> tabelle_produzione.dump
echo   carichi_giacenze                 ; pg_dump --column-inserts -O -x  -h database -t  carichi_giacenze                  mr >> tabelle_produzione.dump
echo   carichi_lista_prodotti_bancalati ; pg_dump --column-inserts -O -x  -h database -t  carichi_lista_prodotti_bancalati  mr >> tabelle_produzione.dump
echo   carichi_prodotti_udc_stor        ; pg_dump --column-inserts -O -x  -h database -t  carichi_prodotti_udc_stor         mr >> tabelle_produzione.dump
echo   carichi_distinte_export          ; pg_dump --column-inserts -O -x  -h database -t  carichi_distinte_export           mr >> tabelle_produzione.dump
echo   carichi_giacenze_stor            ; pg_dump --column-inserts -O -x  -h database -t  carichi_giacenze_stor             mr >> tabelle_produzione.dump
echo   carichi_movimentazioni           ; pg_dump --column-inserts -O -x  -h database -t  carichi_movimentazioni            mr >> tabelle_produzione.dump
echo   carichi_ric_ord_tmp              ; pg_dump --column-inserts -O -x  -h database -t  carichi_ric_ord_tmp               mr >> tabelle_produzione.dump
echo   carichi_distinte_stor            ; pg_dump --column-inserts -O -x  -h database -t  carichi_distinte_stor             mr >> tabelle_produzione.dump
echo   carichi_import_carichi           ; pg_dump --column-inserts -O -x  -h database -t  carichi_import_carichi            mr >> tabelle_produzione.dump
echo   carichi_movimentazioni_stor      ; pg_dump --column-inserts -O -x  -h database -t  carichi_movimentazioni_stor       mr >> tabelle_produzione.dump
echo   carichi_spedizioni_ricevute      ; pg_dump --column-inserts -O -x  -h database -t  carichi_spedizioni_ricevute       mr >> tabelle_produzione.dump
echo   carichi_export_rimanenze         ; pg_dump --column-inserts -O -x  -h database -t  carichi_export_rimanenze          mr >> tabelle_produzione.dump
echo   carichi_lista_giacenze_totali    ; pg_dump --column-inserts -O -x  -h database -t  carichi_lista_giacenze_totali     mr >> tabelle_produzione.dump
echo   carichi_operatori                ; pg_dump --column-inserts -O -x  -h database -t  carichi_operatori                 mr >> tabelle_produzione.dump
echo   carichi_udc                      ; pg_dump --column-inserts -O -x  -h database -t  carichi_udc                       mr >> tabelle_produzione.dump

gzip tabelle_produzione.dump

