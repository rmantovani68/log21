/*
* printfun.c : Stampa Picking List : funzioni di utilita'
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
*
* Copyright AeL srl 1998-2002
*
* 02-01-2002 rm+st : ATTENZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
* 05-02-2002 rm    : riscrittura (da CodeBase a PostgreSQL)
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <glib.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <picking.h>
#include <ep-common.h>
#include <ep-db.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"

BOOL StampaDatiOrdine(char *szOrdProg, char *szPrinterName, BOOL bStampaSuFile)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC=TRUE;
	int nIndex;
	int nTuples;
	BOOL bRetVal=FALSE;
	PGresult *PGResColli;
	PGresult *PGRes;
	BOOL bStorico=FALSE;
	BOOL bOK=TRUE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];
	char szCommand[512];

	strcpy(szRicOrd,"ric_ord");
	strcpy(szOrdProd,"ord_prod");
	strcpy(szColProd,"col_prod");
	strcpy(szRigProd,"rig_prod");

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord where ordprog='%s';",szOrdProg);
	if(DBntuples(PGRes)==0){
		DBclear(PGRes);
		PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select ordprog from ric_ord_stor where ordprog='%s';",szOrdProg);
		if(DBntuples(PGRes)){
			bStorico=TRUE;
			strcpy(szRicOrd,"ric_ord_stor");
			strcpy(szOrdProd,"ord_prod_stor");
			strcpy(szColProd,"col_prod_stor");
			strcpy(szRigProd,"rig_prod_stor");
		} else {
#ifdef TRACE
		trace_out_vstr_date(1, "StampaDatiOrdine: ordine [%s] non trovato ",szOrdProg,szPrinterName);
#endif
		return FALSE;
		}
	}
	DBclear(PGRes);

	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/DatiOrdine_%s",Cfg.szPathStampe,szOrdProg);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		if(WriteTestataDatiOrdine(fp,szOrdProg,bStorico)){

			PGResColli=DBExecQuery(Cfg.nDebugVersion>2,"select cpnmcol from %s where ordprog='%s';",szColProd,szOrdProg);
			if(DBresultStatus(PGResColli)==DBRES_TUPLES_OK){
				if((nTuples=DBntuples(PGResColli))){
					for(nIndex=0;nIndex<nTuples;nIndex++){
						StampaDatiCollo(szOrdProg,nIndex+1, NULL,fp,bStorico);
					}
				} else {
					if(bStorico){
						GetFileString("stampe","ordine", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
					} else {
						GetFileString("stampe","ordine_storico", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
					}
					nRC=PrintListFromSelect(fp,szSelectCmd, szOrdProg,0);
				}
				DBclear(PGResColli);
			}
		}
		/*
		* Salto pagina
		*/
		fprintf(fp,"");

		fclose(fp);
#ifdef TRACE
		trace_out_vstr_date(1, "StampaDatiOrdine (%s - %s)",szOrdProg,szPrinterName);
#endif
		if(nRC){
			sprintf(szTitle,"AeL - Stampa Dati Bolla");
			if(bStampaSuFile){
				sprintf(szCommand,"cp %s %s",szFileName,Cfg.szPathExport);
				system(szCommand);
				
			} else {
				local_PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
			}
			bRetVal=TRUE;
		}
		unlink(szFileName);
	}
	return bRetVal;
}


BOOL StampaDatiCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,BOOL bStorico)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC;
	BOOL bRetVal=FALSE;

	if(szPrinterName==NULL){
		fp=fp_file;
	} else {
		/*
		* Apertura del file per la stampa
		*/
		sprintf(szFileName,"%s/DatiCollo_%s_%03d",Cfg.szPathStampe,szOrdProg,nCollo);
		fp=fopen(szFileName,"w");
		WriteTestataOrdine(fp,szOrdProg,bStorico);
	}
	if(fp){
		if(WriteTestataDatiCollo(fp,szOrdProg,nCollo,bStorico)){

			if(bStorico){
				GetFileString("stampe","collo_storico", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
			} else {
				GetFileString("stampe","collo", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szPrintCfg,NULL); 
			}

			nRC=PrintListFromSelect(fp,szSelectCmd, szOrdProg,nCollo);

			if(szPrinterName!=NULL){
				/*
				* Salto pagina
				*/
				fprintf(fp,"");

				fclose(fp);
#ifdef TRACE
				trace_out_vstr_date(1, "StampaDatiCollo (%s - %d - %s)",szOrdProg,nCollo,szPrinterName);
#endif

				if(nRC){
					sprintf(szTitle,"AeL - Stampa Packing List Collo");
					local_PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
					unlink(szFileName);

					bRetVal=TRUE;
				}
			}
		}
	}
	return bRetVal;
}

BOOL WriteTestataDatiOrdine(FILE *fp,char *szOrdprog,BOOL bStorico)
{
	PGresult *PGRes;
	BOOL bRetVal=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];

	if(bStorico){
		strcpy(szRicOrd,"ric_ord_stor");
		strcpy(szOrdProd,"ord_prod_stor");
		strcpy(szColProd,"col_prod_stor");
		strcpy(szRigProd,"rig_prod_stor");
	} else {
		strcpy(szRicOrd,"ric_ord");
		strcpy(szOrdProd,"ord_prod");
		strcpy(szColProd,"col_prod");
		strcpy(szRigProd,"rig_prod");
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select rodscli,ronmcpe,ronmcll,ropspre,ropsrea,data(rotmrcz),ora(rotmrcz),data(rotmini),ora(rotmini),data(rotmeva),ora(rotmeva) from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(PGRes)==DBRES_TUPLES_OK && (DBntuples(PGRes)==1));

	if (bRetVal){
		fprintf(fp,"==========================================================\n");
		fprintf(fp,"                Ordine : %s\n",szOrdprog);
		fprintf(fp,"==========================================================\n\n");
		
		fprintf(fp,"Cliente : %s\n",DBgetvalue(PGRes,0,0));
		fprintf(fp,"Copie : %d\n",atoi(DBgetvalue(PGRes,0,1)));
		fprintf(fp,"Colli : %d\n\n",atoi(DBgetvalue(PGRes,0,2)));
		
		fprintf(fp,"Peso calcolato : %.3f Kg.\n",(float)((float)atoi(DBgetvalue(PGRes,0,3))/(float)1000));
		fprintf(fp,"Peso reale     : %.3f Kg.\n\n",(float)((float)atoi(DBgetvalue(PGRes,0,4))/(float)1000));
		
		fprintf(fp,"Ricezione          : %s %s\n",DBgetvalue(PGRes,0,5),DBgetvalue(PGRes,0,5));
		fprintf(fp,"Inizio lavorazione : %s %s\n",DBgetvalue(PGRes,0,6),DBgetvalue(PGRes,0,6));
		fprintf(fp,"Evasione           : %s %s\n\n",DBgetvalue(PGRes,0,7),DBgetvalue(PGRes,0,7));
		fprintf(fp,"----------------------------------------------------------\n\n");
	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "WriteTestataOrdine(%s) -> fallita select",szOrdprog);
#endif
	}
	DBclear(PGRes);

	return bRetVal;
}

BOOL WriteTestataDatiCollo(FILE *fp,char *szOrdprog, int nCollo,BOOL bStorico)
{
	PGresult *PGRes;
	PGresult *PGResCollo;
	BOOL bRetVal=FALSE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];
	int nIndex;
	int nTuples;

	if(bStorico){
		strcpy(szRicOrd,"ric_ord_stor");
		strcpy(szOrdProd,"ord_prod_stor");
		strcpy(szColProd,"col_prod_stor");
		strcpy(szRigProd,"rig_prod_stor");
	} else {
		strcpy(szRicOrd,"ric_ord");
		strcpy(szOrdProd,"ord_prod");
		strcpy(szColProd,"col_prod");
		strcpy(szRigProd,"rig_prod");
	}

	PGRes=DBExecQuery(Cfg.nDebugVersion>2,"select * from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(PGRes)==DBRES_TUPLES_OK && (DBntuples(PGRes)==1));

	if (bRetVal){
		fprintf(fp,"\nCollo %d\n",nCollo);
		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select cpnmrgh,cpnmcpe,cppspre,cppsrea,data(cptmeva),ora(cptmeva) from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK){
			fprintf(fp,"   Righe: %d      Copie: %d\n",atoi(DBgetvalue(PGResCollo,0,0)),atoi(DBgetvalue(PGResCollo,0,1)));
			fprintf(fp,"   Peso Predeterminato : %.3f Kg\n",(float)((float)atoi(DBgetvalue(PGResCollo,0,2))/(float)1000));
			fprintf(fp,"   Peso Reale          : %.3f Kg\n",(float)((float)atoi(DBgetvalue(PGResCollo,0,3))/(float)1000));
			fprintf(fp,"   Evasione            : %s %s\n",DBgetvalue(PGResCollo,0,4),DBgetvalue(PGResCollo,0,5));
		}
		DBclear(PGResCollo);

		/*
		* controllo l'eventualita' che il collo sia transitato in bilancia e scartato (FUORI TOLLERANZA opp FUORI LINEA) 
		*/
		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select data(cptmpes),ora(cptmpes),cppsrea from colli_scartati where ordprog='%s' and cpnmcol=%d;",szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK){
			if((nTuples=DBntuples(PGResCollo))){
				fprintf(fp,"   Scartato %d volte:\n",nTuples);
				for(nIndex=0;nIndex<nTuples;nIndex++){
					fprintf(fp,"      - %s %s con peso %.3f Kg\n",DBgetvalue(PGResCollo,nIndex,0),DBgetvalue(PGResCollo,nIndex,1),(float)((float)atoi(DBgetvalue(PGResCollo,nIndex,2))/(float)1000));
				}
			}
		}
		DBclear(PGResCollo);
		
		PGResCollo=DBExecQuery(Cfg.nDebugVersion>2,"select data(tmread),ora(tmread),bnnmbnc from storico_colli_bancale scb,storico_bancali sb where ordprog='%s' and cpnmcol=%d and scb.key_bancale=sb.key_bancale;",szOrdprog,nCollo);
		if(DBresultStatus(PGResCollo)==DBRES_TUPLES_OK){
			if((nTuples=DBntuples(PGResCollo))){
				fprintf(fp,"   Bancale: %d\n",atoi(DBgetvalue(PGResCollo,0,2)));
				fprintf(fp,"   Controllo:%s %s \n",DBgetvalue(PGResCollo,0,0),DBgetvalue(PGResCollo,0,1));
			} else {
				fprintf(fp,"   Nessun dato di pallettizzazione\n");
			}
			
		}
		DBclear(PGResCollo);

	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "WriteTestataCollo(%s - %d) -> fallita select",szOrdprog,nCollo);
#endif
	}

	DBclear(PGRes);

	return bRetVal;
}
