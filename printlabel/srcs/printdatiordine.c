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
#include <gio/gio.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"

ep_bool_t StampaDatiOrdine(char *szOrdProg, char *szPrinterName, ep_bool_t bStampaSuFile)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC=TRUE;
	int nIndex;
	int nTuples;
	ep_bool_t bRetVal=FALSE;
	DBResult *DBResColli;
	DBResult *DBRes;
	ep_bool_t bStorico=FALSE;
	ep_bool_t bOK=TRUE;
	char szRicOrd[128];
	char szOrdProd[128];
	char szColProd[128];
	char szRigProd[128];
	char szCommand[512];

	strcpy(szRicOrd,"ric_ord");
	strcpy(szOrdProd,"ord_prod");
	strcpy(szColProd,"col_prod");
	strcpy(szRigProd,"rig_prod");

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord where ordprog='%s';",szOrdProg);
	if(DBntuples(DBRes)==0){
		DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select ordprog from ric_ord_stor where ordprog='%s';",szOrdProg);
		if(DBntuples(DBRes)){
			bStorico=TRUE;
			strcpy(szRicOrd,"ric_ord_stor");
			strcpy(szOrdProd,"ord_prod_stor");
			strcpy(szColProd,"col_prod_stor");
			strcpy(szRigProd,"rig_prod_stor");
		} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "StampaDatiOrdine: ordine [%s] non trovato ",szOrdProg,szPrinterName);
#endif
		return FALSE;
		}
	}
	DBclear(DBRes);

	/*
	* Apertura del file per la stampa
	*/
	sprintf(szFileName,"%s/DatiOrdine_%s",Cfg.szPathStampe,szOrdProg);
	if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
		if(WriteTestataDatiOrdine(fp,szOrdProg,bStorico)){

			DBResColli=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmcol from %s where ordprog='%s';",szColProd,szOrdProg);
			if(DBresultStatus(DBResColli)==DBRES_TUPLES_OK){
				if((nTuples=DBntuples(DBResColli))){
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
				DBclear(DBResColli);
			}
		}
		/*
		* Salto pagina
		*/
		fprintf(fp,"");

		fclose(fp);
#ifdef TRACE
		trace_debug(TRUE, TRUE, "StampaDatiOrdine (%s - %s)",szOrdProg,szPrinterName);
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


ep_bool_t StampaDatiCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,ep_bool_t bStorico)
{
	char szSelectCmd[2048];
	FILE *fp;
	char szTitle[128];
	char szFileName[128];
	int nRC;
	ep_bool_t bRetVal=FALSE;

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
				trace_debug(TRUE, TRUE, "StampaDatiCollo (%s - %d - %s)",szOrdProg,nCollo,szPrinterName);
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

ep_bool_t WriteTestataDatiOrdine(FILE *fp,char *szOrdprog,ep_bool_t bStorico)
{
	DBResult *DBRes;
	ep_bool_t bRetVal=FALSE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select rodscli,ronmcpe,ronmcll,ropspre,ropsrea,data(rotmrcz),ora(rotmrcz),data(rotmini),ora(rotmini),data(rotmeva),ora(rotmeva) from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(DBRes)==DBRES_TUPLES_OK && (DBntuples(DBRes)==1));

	if (bRetVal){
		fprintf(fp,"==========================================================\n");
		fprintf(fp,"                Ordine : %s\n",szOrdprog);
		fprintf(fp,"==========================================================\n\n");
		
		fprintf(fp,"Cliente : %s\n",DBgetvalue(DBRes,0,0));
		fprintf(fp,"Copie : %d\n",atoi(DBgetvalue(DBRes,0,1)));
		fprintf(fp,"Colli : %d\n\n",atoi(DBgetvalue(DBRes,0,2)));
		
		fprintf(fp,"Peso calcolato : %.3f Kg.\n",(float)((float)atoi(DBgetvalue(DBRes,0,3))/(float)1000));
		fprintf(fp,"Peso reale     : %.3f Kg.\n\n",(float)((float)atoi(DBgetvalue(DBRes,0,4))/(float)1000));
		
		fprintf(fp,"Ricezione          : %s %s\n",DBgetvalue(DBRes,0,5),DBgetvalue(DBRes,0,5));
		fprintf(fp,"Inizio lavorazione : %s %s\n",DBgetvalue(DBRes,0,6),DBgetvalue(DBRes,0,6));
		fprintf(fp,"Evasione           : %s %s\n\n",DBgetvalue(DBRes,0,7),DBgetvalue(DBRes,0,7));
		fprintf(fp,"----------------------------------------------------------\n\n");
	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "WriteTestataOrdine(%s) -> fallita select",szOrdprog);
#endif
	}
	DBclear(DBRes);

	return bRetVal;
}

ep_bool_t WriteTestataDatiCollo(FILE *fp,char *szOrdprog, int nCollo,ep_bool_t bStorico)
{
	DBResult *DBRes;
	DBResult *DBResCollo;
	ep_bool_t bRetVal=FALSE;
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

	DBRes=DBExecQuery(Cfg.nDebugLevel>2,"select * from %s where ordprog='%s';",szRicOrd,szOrdprog);

	bRetVal=(DBresultStatus(DBRes)==DBRES_TUPLES_OK && (DBntuples(DBRes)==1));

	if (bRetVal){
		fprintf(fp,"\nCollo %d\n",nCollo);
		DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select cpnmrgh,cpnmcpe,cppspre,cppsrea,data(cptmeva),ora(cptmeva) from %s where ordprog='%s' and cpnmcol=%d;",szColProd,szOrdprog,nCollo);
		if(DBresultStatus(DBResCollo)==DBRES_TUPLES_OK){
			fprintf(fp,"   Righe: %d      Copie: %d\n",atoi(DBgetvalue(DBResCollo,0,0)),atoi(DBgetvalue(DBResCollo,0,1)));
			fprintf(fp,"   Peso Predeterminato : %.3f Kg\n",(float)((float)atoi(DBgetvalue(DBResCollo,0,2))/(float)1000));
			fprintf(fp,"   Peso Reale          : %.3f Kg\n",(float)((float)atoi(DBgetvalue(DBResCollo,0,3))/(float)1000));
			fprintf(fp,"   Evasione            : %s %s\n",DBgetvalue(DBResCollo,0,4),DBgetvalue(DBResCollo,0,5));
		}
		DBclear(DBResCollo);

		/*
		* controllo l'eventualita' che il collo sia transitato in bilancia e scartato (FUORI TOLLERANZA opp FUORI LINEA) 
		*/
		DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select data(cptmpes),ora(cptmpes),cppsrea from colli_scartati where ordprog='%s' and cpnmcol=%d;",szOrdprog,nCollo);
		if(DBresultStatus(DBResCollo)==DBRES_TUPLES_OK){
			if((nTuples=DBntuples(DBResCollo))){
				fprintf(fp,"   Scartato %d volte:\n",nTuples);
				for(nIndex=0;nIndex<nTuples;nIndex++){
					fprintf(fp,"      - %s %s con peso %.3f Kg\n",DBgetvalue(DBResCollo,nIndex,0),DBgetvalue(DBResCollo,nIndex,1),(float)((float)atoi(DBgetvalue(DBResCollo,nIndex,2))/(float)1000));
				}
			}
		}
		DBclear(DBResCollo);
		
		DBResCollo=DBExecQuery(Cfg.nDebugLevel>2,"select data(tmread),ora(tmread),bnnmbnc from storico_colli_bancale scb,storico_bancali sb where ordprog='%s' and cpnmcol=%d and scb.key_bancale=sb.key_bancale;",szOrdprog,nCollo);
		if(DBresultStatus(DBResCollo)==DBRES_TUPLES_OK){
			if((nTuples=DBntuples(DBResCollo))){
				fprintf(fp,"   Bancale: %d\n",atoi(DBgetvalue(DBResCollo,0,2)));
				fprintf(fp,"   Controllo:%s %s \n",DBgetvalue(DBResCollo,0,0),DBgetvalue(DBResCollo,0,1));
			} else {
				fprintf(fp,"   Nessun dato di pallettizzazione\n");
			}
			
		}
		DBclear(DBResCollo);

	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "WriteTestataCollo(%s - %d) -> fallita select",szOrdprog,nCollo);
#endif
	}

	DBclear(DBRes);

	return bRetVal;
}
