#!/bin/awk -f

#
# script per verificare le query sql con tempi di esecuzione
# maggiori di un tempo stabilito
#
BEGIN {
	exec=0
	tempo_exec=0
}

{ 
	ora=$2
	comando=$4
	comando_1=$5
	comando_2=$6
	ore=substr(ora,2,2)
	minuti=substr(ora,5,2)
	secondi=substr(ora,8,2)
	millisecondi=substr(ora,11,3)

	if(exec==1 && (comando=="Destinazione" )){
		ore_passati=ore-ore_start
		minuti_passati=minuti-minuti_start
		secondi_passati=secondi-secondi_start
		millisecondi_passati=millisecondi-millisecondi_start

		if(ore_passati>=1){
			minuti_passati=(60+minuti)-minuti_start;
			ore_passati=ore_passati-1;
		}

		if(minuti_passati>=1){
			secondi_passati=(60+secondi)-secondi_start;
			minuti_passati=minuti_passati-1;
		}

		if(secondi_passati>=1){
			if(millisecondi>millisecondi_start){
				millisecondi_passati=millisecondi-millisecondi_start
			} else {
				millisecondi_passati=(1000+millisecondi)-millisecondi_start
				secondi_passati=secondi_passati-1;
			}
		}

		if(ore_passati*3600000+minuti_passati*60000+secondi_passati*1000+millisecondi_passati>=tempo_exec){
			printf "comando   : %s\n",exec_sql 
			printf "tempo di esecuzione : %d:%d:%d.%03d\n",ore_passati,minuti_passati,secondi_passati,millisecondi_passati
	#	} else {
	#		printf "comando   : %s\n",exec_sql 
	#		printf "tempo di esecuzione : %d.%03d\n",secondi_passati,millisecondi_passati
		}
		exec=0
	}

	if(comando=="Started"){
	#	printf "comando : %s\n",$0
	#	printf "ora inizio : %s\n",ora 
		exec_sql=$0;
		ore_start=substr(ora,2,2)
		minuti_start=substr(ora,5,2)
		secondi_start=substr(ora,8,2)
		millisecondi_start=substr(ora,11,3)
		exec=1
	}
}
