#!/bin/bash
echo "Consuntivo scarti presenti in lista lavorazione\n" > ../export/scarti-group.txt
echo 'select sscdpro as PRODOTTO,sum(ssnmcpe) as COPIE,sscdflg as FLAG from storico_scarti where sscdflg!=\'NON PRESENTE\' group by sscdpro,sscdflg order by sscdflg desc,count(sscdpro) desc;' | psql -h scarti scarti >> ../export/scarti-group.txt
echo "\nTotale scarti presenti in lista lavorazione\n" >> ../export/scarti-group.txt
echo 'select sum(ssnmcpe) as COPIE,sscdflg as FLAG from storico_scarti where sscdflg!=\'NON PRESENTE\' group by sscdflg ;' | psql -h scarti scarti >> ../export/scarti-group.txt

echo "\n\nConsuntivo scarti non presenti in lista lavorazione\n" > ../export/scarti-group.txt
echo 'select sscdpro as PRODOTTO,sum(ssnmcpe) as COPIE,sscdflg as FLAG from storico_scarti where sscdflg=\'NON PRESENTE\' group by sscdpro,sscdflg order by sscdflg desc,count(sscdpro) desc;' | psql -h scarti scarti >> ../export/scarti-group.txt
echo "\nTotale scarti non presenti in lista lavorazione\n" >> ../export/scarti-group.txt
echo 'select sum(ssnmcpe) as COPIE,sscdflg as FLAG from storico_scarti where sscdflg=\'NON PRESENTE\' group by sscdflg ;' | psql -h scarti scarti >> ../export/scarti-group.txt

#echo 'select sum(ssnmcpe) as COPIE,sscdflg as FLAG from storico_scarti group by sscdflg ;' | psql -h scarti scarti >> ../export/scarti-group.txt
echo "\n\nTotale scarti presenti e non in lista lavorazione\n" >> ../export/scarti-group.txt
echo 'select sum(ssnmcpe) as COPIE from storico_scarti;' | psql -h scarti scarti >> ../export/scarti-group.txt

