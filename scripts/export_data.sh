#
# rm 28-05-2003
# export dati
#

DATA=`zenity --title "Export Dati a HOST" --text "Seleziona il dato da esportare" --list --radiolist --width=350 --height=350 --column "!" --column "TABELLA" TRUE Colli FALSE Ubicazioni`
#echo $?
#echo data=$DATA
case $DATA in
	Colli) 
		echo Colli
		# echo 'select r.ordprog,cpnmcol,cppspre,cppsrea,cppspre, cppsrea, cptpfor, cpnmrgh, cpnmcpe, cpbrcde, rocdlin from ric_ord r, col_prod c where r.ordprog=c.ordprog order by ordprog,cpnmcol;' | psql rcs > /u/prj/sli/export/col_stor.txt
		# rm 17-07-2003 : nuova versione 
		echo 'select r.ordprog as "ORDINE",cpnmcol as "NUMERO COLLO",cppspre as "PESO TEORICO", cppsrea as "PESO REALE", cppspre-cppsrea as "DIFFERENZA PESO", cptpfor as "FORMATO", cpnmrgh as "RIGHE", cpnmcpe as "PEZZI", cpbrcde as "BARCODE", rocdlin as "LINEA",cpprgcl as "SEGNACOLLO",cpcdflg as "RETTIFICA",cpcntsc as "SCARTATO" from ric_ord r, col_prod c where r.ordprog=c.ordprog order by c.ordprog,cpnmcol;' | psql rcs > /u/prj/sli/export/col_stor.txt
		zenity --info --title "Export Dati a HOST" --text "Export tabella COLLI terminato" --width=350 --height=350
		;;
	Ubicazioni) 
		echo Ubicazioni;
		echo 'select codprod,ubicazione_ridotta(ubicazione),cnistato,prswffo,prqtpcf,prlungh,prlargh,praltez,prpesgr from ubicazioni u, catalogo c where u.codprod=c.prcdpro order by ubicazione_ridotta(ubicazione);' | psql rcs > /u/prj/sli/export/ubicazioni.txt
		zenity --info --title "Export Dati a HOST" --text "Export tabella UBICAZIONI terminato" --width=350 --height=350
		;;
	*) :;;
esac

