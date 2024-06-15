/*
* Modulo : ep-db.c
* -----------------------
* Funzioni di utilità per Easy Picking
*
* Data creazione : Fri May 31 17:33:07 CEST 2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <trace.h>

#include <pmx_msq.h>

#include <glib.h>
#include <gio/gio.h>

#include <proc_list.h>
#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>

static DBconn *DB=NULL;
static GArray *tTablesArray=NULL;


/*
* Normalizza il fastidiosissimo apice (per PGSQL !)
*/
char *NormalizeString(char *szString)
{
	static char szDestination[1024];
	char szTmpString[1024];
	char *pPtr;


	/*
	* sostituzione di eventuale '\' alla fine della stringa 
	* per evitare che se poi ci metto l'apice la query si incasini
	*/
	if (strlen(szString)>0){
		if (szString[strlen(szString)-1]=='\\'){
			szString[strlen(szString)-1]=' ';
		}
	}


	/*
	* sostituzione del (') 
	* rm 31-07-2013 : se trovo \' non faccio niente 
	*/ 
	strcpy(szDestination,szString);
	pPtr=szDestination;
	while ((pPtr=strchr(pPtr,'\''))!=(char *)NULL){
		/*
		* Tronco la stringa sull'apice
		*/
		*pPtr = '\0';
		strcpy(szTmpString,szDestination);
		strcat(szTmpString,"''");
		strcat(szTmpString,(pPtr+1));

		/*
		* Riporto la modifica
		*/
		strcpy(szDestination,szTmpString);
		pPtr+=2;
	}


	/*
	*   * Sostituzione del % che da problemi sul vsprintf
	*     */
	pPtr=szDestination;
	while ((pPtr=strchr(pPtr,'%'))!=(char *)NULL){
		/*
		*     * Tronco la stringa sull'apice
		*         */
		*pPtr = '\0';
		strcpy(szTmpString,szDestination);
		strcat(szTmpString,"P");
		strcat(szTmpString,(pPtr+1));

		/*
		*     * Riporto la modifica
		*         */
		strcpy(szDestination,szTmpString);
		pPtr+=2;
	}

	return (szDestination);
}





/*
* ep_bool_t GetAsciiRecord(szBuffer,pDB)
* Legge dal buffer i dati da inserire nella tabella 
* rm + st : 16-06-2000 : portata su PGSQL
*/
ep_bool_t GetAsciiRecord(char *szBuffer,PDBSTRUCT pDB)
{
	int nFieldPosition;
	int nLineNumber;
	int nFieldIndex;
	int nFieldLen;
	int nFirst;
	char szFieldString[128];
	char szTmp[128];
	char szSQLCmd[2048];
	int nLen;
	int nIndex;
	int nTmpIndex;
	DBresult *DBRes;
	ep_bool_t bTrace=FALSE;

#ifdef TRACE_SQL
	bTrace=TRUE;
#endif

	/*
	* controllo lunghezza record
	*/
	if(strlen(szBuffer)<(unsigned int)pDB->nRecordLen){
		return FALSE;
	}

	DeleteNewLine(szBuffer);
	nLen=strlen(szBuffer);

	if(nLen >= pDB->nRecordLen && !strncmp(szBuffer+pDB->nRecordTypePos,pDB->szRecordType,pDB->nRecordTypeLen)){
		nLineNumber=0;

		/*
		* rm 19-01-1999 : gestione multilinea
		*/
		if(pDB->nLinesNumber>1){
			nLineNumber=szBuffer[pDB->nLineNumberPos]-'0';
		}

		/*
		* Comando SQL di inserimento dati in tabella
		*/
		sprintf(szSQLCmd,"INSERT INTO %s (",pDB->szTableName);
		nFirst=0;
		for(nFieldIndex=0; nFieldIndex < pDB->nFieldsNumber; nFieldIndex++){
			if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
				strcat(szSQLCmd, nFirst?",":"");
				strcat(szSQLCmd,pDB->pFields[nFieldIndex]->szFieldName);
				nFirst=1;
			}
		}
		strcat(szSQLCmd,") VALUES (");
		/*
		* Aggiunta di una riga alla tabella
		*/
		nFirst=0;
		nFieldPosition=pDB->nDataStart;
		for(nFieldIndex=0; nFieldIndex < pDB->nFieldsNumber; nFieldIndex++){
			if(pDB->pFields[nFieldIndex]->nLineNumber==(unsigned int)nLineNumber){

				/* calcolo la lunghezza  del campo */
				nFieldLen=pDB->pFields[nFieldIndex]->nFieldLen+(pDB->pFields[nFieldIndex]->nFieldDec ? pDB->pFields[nFieldIndex]->nFieldDec+1 : 0 );

				/*
				* rm 19-01-1999 : gestione offset
				*/
				if(pDB->pFields[nFieldIndex]->nFieldOffset){
					nFieldPosition=pDB->pFields[nFieldIndex]->nFieldOffset;
				}

				if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
					/* copio il dato */
					strncpy(szFieldString, szBuffer+nFieldPosition, nFieldLen);
					if(!strlen(szFieldString)) strcpy(szFieldString," ");
					szFieldString[nFieldLen]='\0';

					/*
					* Comando SQL
					*/
					strcat(szSQLCmd, nFirst?",":"");
					switch(pDB->pFields[nFieldIndex]->cFieldType){
						case 'N': /* numero  */
						case 'P': /* numero  */
							strcat (szSQLCmd, szFieldString);
							break;
						case 'C': /* carattere */
							nIndex=0;
							nTmpIndex=0;
							while(szFieldString[nIndex]){
								if(szFieldString[nIndex]=='\''){
									szTmp[nTmpIndex]=szFieldString[nIndex];
									nTmpIndex++;
								}
								szTmp[nTmpIndex]=szFieldString[nIndex];
								nIndex++;
								nTmpIndex++;
							}
							szTmp[nIndex]='\0';
							strcat (szSQLCmd, "'");
							strcat (szSQLCmd, szTmp);
							strcat (szSQLCmd, "'");
							break;
					}
					nFirst=1;
				} 
				/* calcolo la posizione del campo */
				nFieldPosition+=nFieldLen;
			}
		}
		strcat(szSQLCmd,");");
		DBRes=DBExecQuery(bTrace,szSQLCmd);
		DBclear(DBRes);
		return TRUE;
	}
	return FALSE;
}


/*
* WriteAsciiRecord()
* scrittura di un record in formato ascii
* bFillZero : i campi numerici vengono 'riempiti' con '0' a sinistra, es: 23 (6 cifre) -> 000023
* bSigned : i campi numerici iniziano con il segno, es: 56 (6 cifre) -> +00056 
*/
ep_bool_t WriteAsciiRecord(FILE *fp,PDBSTRUCT pDB,DBresult *DBRes,int nIndex,ep_bool_t bFillZero,ep_bool_t bSigned)
{
	char szRecord[4096];
	char szBuffer[2048];
	PEP_FIELD pField;
	int nFieldIndex;
	int nFieldPosition;
	int nFieldLen;
	int nRecordLen=0;
	int nFieldsNumber=0;

	nFieldsNumber=DBnfields(DBRes);

	/*
	* Ricavo la lunghezza del buffer di scrittura
	*/


	nRecordLen=pDB->nRecordLen;
	if(nRecordLen==0){
		nRecordLen=0;
		for(nFieldIndex=0;nFieldIndex<nFieldsNumber;nFieldIndex++){
			if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
				nRecordLen+=pDB->pFields[nFieldIndex]->nFieldLen;
			}
		}
	}
	nRecordLen+=pDB->nRecordTypeLen;

	memset(szRecord,' ',nRecordLen);
	szRecord[nRecordLen+0] = '\r';
	szRecord[nRecordLen+1] = '\n';
	szRecord[nRecordLen+2] = '\0';

	if(pDB->nRecordTypeLen){
		memcpy(szRecord+pDB->nRecordTypePos,pDB->szRecordType,pDB->nRecordTypeLen);
	}

	/* 
	* riempio il buffer con i dati del record 
	*/
	nFieldPosition=pDB->nDataStart;
	for(nFieldIndex=0;nFieldIndex<nFieldsNumber;nFieldIndex++){
		pField=pDB->pFields[nFieldIndex];
		if(pField->nFieldFlag & EXPORT_MASK){
			/*
			* calcolo la lunghezza del campo 
			*/
			nFieldLen=pField->nFieldLen;
			/*
			* Esporto il campo
			*/
			switch(pField->cFieldType){
				default:
					/* anche i campi data */
				case 'C':
				case 'c':
					sprintf(szBuffer, "%-*s", nFieldLen,DBgetvalue(DBRes,nIndex,nFieldIndex));
				break;
				case 'N':
				case 'n':
				case 'P':
				case 'p':
					if(bFillZero){
						sprintf(szBuffer, "%0*d", nFieldLen,atoi(DBgetvalue(DBRes,nIndex,nFieldIndex)));
					} else {
						sprintf(szBuffer, "%*d", nFieldLen,atoi(DBgetvalue(DBRes,nIndex,nFieldIndex)));
					}
					if(bSigned){
						szBuffer[0]=atoi(DBgetvalue(DBRes,nIndex,nFieldIndex))>=0?'+':'-';
					}
				break;
			}
			memcpy(szRecord+nFieldPosition, szBuffer, nFieldLen);
			nFieldPosition+=nFieldLen;
		}
	}
	/*
	* rm 19-02-2003 : uso fputs per problemi con catatteri particolari (%)
	* fprintf(fp,szRecord);
	*/
	fputs(szRecord,fp);

	return(TRUE);
}


/*
* IsAsciiRecord(szBuffer,pDB)
*/
ep_bool_t IsAsciiRecord(char *szBuffer,PDBSTRUCT pDB)
{
	int nLen;

	DeleteNewLine(szBuffer);
	nLen=strlen(szBuffer);

	if(nLen >= pDB->nRecordLen && !strncmp(szBuffer+pDB->nRecordTypePos,pDB->szRecordType,pDB->nRecordTypeLen)){
		return TRUE;
	}
	return FALSE;
}

/*
* ep_bool_t UpdateAsciiRecord(char *szBuffer,PDBSTRUCT pDB, ep_bool_t bUpdate)
* + st  02/10/2000 
* Legge i dati da szBuffer:
* - se bUpdate=TRUE effettua un ricerca nella tabella utilizzando la chiave "primaria -> UNIQUE e COSTITUITA DA UN SOLO CAMPO" 
*              (la prima indicata nel file di configurazione .cfg) effettuando un update se trova gia' presente un riga; altrimenti inserisce;
* - se bUpdate=FALSE inserisce direttamente in tabella (ATTENZIONE ALLE CHIAVI "primarie");
* + rm  08/08/2001 
* gestione indice composto da piu' campi
* + rm  04/05/2004 
* in caso di errore in inserimento/update restituisce il codice errore
*/
int UpdateAsciiRecord(char *szBuffer, PDBSTRUCT pDB, ep_bool_t bUpdate)
{
	int nFieldPosition;
	int nFieldIndex;
	int nIndex;
	int nLen;
	int nFirst;
	char szFieldString[256];
	char szUpdateSQLCmd[2048];
	char szInsertSQLCmd[2048];
	char *pszKeyFieldsData[256];
	int nKeyFields[256];
	int nKeyFieldsNumber=0;
	char *pszBuffer;
	char szTmpBuffer[256];
	char *pPtr;
	char  *szSeparator="+";
	DBresult *DBRes;
	DBresult *DBResInsert;
	ep_bool_t bTrace=FALSE;
	int nResult=TRUE;

#ifdef TRACE_SQL
	bTrace=TRUE;
#endif

	DeleteNewLine(szBuffer);
	nLen=strlen(szBuffer);

	/*
	* Verifico corrispondenza tipo record
	*/
	if(nLen >= pDB->nRecordLen && !strncmp(szBuffer+pDB->nRecordTypePos,pDB->szRecordType,pDB->nRecordTypeLen)){
		/*
		* inserimento dei dati in tabelle SQL
		*/
		/*
		* Se Update allora genero la lista dei campi chiave
		*/
		if(bUpdate){
			if(pDB->Tags==(PTAG)NULL){
				fprintf(stderr,"Errore : Overwrite e Indice [0] non presente !!!\n");
#ifdef TRACE
				trace_debug(TRUE, TRUE, "UpdateAsciiRecord(%s) : Errore : Overwrite e Indice [0] non presente !!!" , pDB->szTableName);
#endif
				return FALSE;
			}

			/*
			* Genero la lista campi chiave (solo del primo indice)
			*/
			nKeyFieldsNumber=0;
			strcpy(szTmpBuffer,pDB->Tags[0].szExpression);
			pszBuffer=szTmpBuffer;
			while((pPtr=strtok(pszBuffer,szSeparator))!=NULL){
				for(nFieldIndex=0; nFieldIndex<pDB->nFieldsNumber; nFieldIndex++){
					if(!strcmp(pPtr, pDB->pFields[nFieldIndex]->szFieldName)){
						nKeyFields[nKeyFieldsNumber]=nFieldIndex;
						nKeyFieldsNumber++;
						break;
					}
				}
				pszBuffer=NULL;
			}

			if(nKeyFieldsNumber==0){
				fprintf(stderr,"Errore : Overwrite attivo e campo indice non presente [%s]\n",pDB->Tags[0].szExpression);
#ifdef TRACE
				trace_debug(TRUE, TRUE, "UpdateAsciiRecord(%s) : Errore : Overwrite attivo e campo indice non presente [%s]" , pDB->szTableName,pDB->Tags[0].szExpression);
#endif
				return FALSE;
			}
		}

		/*
		* aggiungo un record al DataBase 
		*/
		if (bUpdate){
			strcpy (szUpdateSQLCmd,"UPDATE " );
			strcat (szUpdateSQLCmd, pDB->szTableName);
			strcat (szUpdateSQLCmd," SET "); 
		}

		sprintf(szInsertSQLCmd,"INSERT INTO %s (",pDB->szTableName);
		nFirst=0;
		for(nFieldIndex=0; nFieldIndex < pDB->nFieldsNumber; nFieldIndex++){
			if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
				strcat(szInsertSQLCmd, nFirst?",":"");
				strcat(szInsertSQLCmd,pDB->pFields[nFieldIndex]->szFieldName);
				nFirst=1;
			}
		}
		strcat (szInsertSQLCmd,") VALUES ( "); 

		nFieldPosition=pDB->nDataStart;

		for(nFieldIndex=0; nFieldIndex<pDB->nFieldsNumber; nFieldIndex++){
			/*
			* gestione offset
			*/
			if(pDB->pFields[nFieldIndex]->nFieldOffset){
				nFieldPosition=pDB->pFields[nFieldIndex]->nFieldOffset;
			}
			if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
				/* copio il dato */
				strncpy(szFieldString, szBuffer+nFieldPosition, pDB->pFields[nFieldIndex]->nFieldLen);
				szFieldString[pDB->pFields[nFieldIndex]->nFieldLen]='\0';

				StrNormalize(szFieldString);


				/*
				* Conversione per il valori numerici
				*/
				if(pDB->pFields[nFieldIndex]->cFieldType=='N'){
					if(pDB->pFields[nFieldIndex]->nFieldDec){
						sprintf(szFieldString,"%0f",strtod(szFieldString,NULL));
					} else {
						sprintf(szFieldString,"%0d",atoi(szFieldString));
					}
				}

				/*
				* se si tratta di un campo chiave (e sono in update) memorizzo il valore
				*/
				if(bUpdate){
					for(nIndex=0;nIndex<nKeyFieldsNumber;nIndex++){
						if(nFieldIndex==nKeyFields[nIndex]) {
							pszKeyFieldsData[nIndex]=strdup(szFieldString);
						}
					}
				}

				if (bUpdate){
					strcat (szUpdateSQLCmd, nFieldIndex?", ":" ");
				}
				strcat (szInsertSQLCmd, nFieldIndex?", ":" ");

				/*
				* Se sono in overwrite inserisco il nome del campo prima 
				* della assegnazione a valore
				*/
				if (bUpdate){
					strcat (szUpdateSQLCmd, pDB->pFields[nFieldIndex]->szFieldName);
					strcat (szUpdateSQLCmd, "=");
				}

				/* 
				* inserimento valore attributo
				*/
				if(pDB->pFields[nFieldIndex]->cFieldType=='C'){
					if (bUpdate){
						strcat (szUpdateSQLCmd, "\'");
					}
					strcat (szInsertSQLCmd, "\'");
				}
				/*
				* sostituzione del (') 
				*/ 
				strcpy(szFieldString,NormalizeString(szFieldString));

				if (bUpdate){
					strcat (szUpdateSQLCmd, szFieldString);
				}
				strcat (szInsertSQLCmd, szFieldString);

				if(pDB->pFields[nFieldIndex]->cFieldType=='C'){
					if (bUpdate){
						strcat (szUpdateSQLCmd, "\'");
					}
					strcat (szInsertSQLCmd, "\'");
				}
			
			}
			nFieldPosition+=pDB->pFields[nFieldIndex]->nFieldLen;
		}
		/*
		* in overwrite inserisco la condizione di ricerca
		*/
		if (bUpdate){
			strcat (szUpdateSQLCmd, " WHERE (");
			for(nIndex=0;nIndex<nKeyFieldsNumber;nIndex++){
				if(nIndex){
					strcat (szUpdateSQLCmd, " AND ");
				}
				strcat (szUpdateSQLCmd, pDB->pFields[nKeyFields[nIndex]]->szFieldName);
				strcat (szUpdateSQLCmd, " = ");

				if(pDB->pFields[nKeyFields[nIndex]]->cFieldType=='C'){
					strcat (szUpdateSQLCmd, "\'");
				}

				strcat (szUpdateSQLCmd, pszKeyFieldsData[nIndex]);

				if(pDB->pFields[nKeyFields[nIndex]]->cFieldType=='C'){
					strcat (szUpdateSQLCmd, "\'");
				}

			}
			strcat (szUpdateSQLCmd, " ) ");


		}
		if (bUpdate){
			strcat (szUpdateSQLCmd, ";");
		}
		strcat (szInsertSQLCmd, ");");

		if (bUpdate){
			/*
			* con bUpdate=TRUE -> se DBcmdTuples ritorna 1 
			* - la riga era gia'presente ed e' stata aggiornata;
			* - altrimenti significa che la riga non e' presente e va inserita
			*/
#ifdef TRACE_ELIMINATO
			trace_debug(TRUE, TRUE, "Exec Query : %s",szUpdateSQLCmd);
#endif
			DBRes =  DBExecQuery(bTrace,szUpdateSQLCmd);
			if(DBresultStatus(DBRes)==DBRES_FATAL_ERROR){
				nResult=DBRES_FATAL_ERROR;
			}
			/*
			* Update fallito a causa di record non presente (lo inserisco)
			*/
			if (atoi(DBcmdTuples(DBRes))==0){
				DBResInsert = DBExecQuery(bTrace, szInsertSQLCmd);
				if(DBresultStatus(DBResInsert)==DBRES_FATAL_ERROR){
					nResult=DBRES_FATAL_ERROR;
				}
				DBclear(DBResInsert);
			}
			DBclear(DBRes);
		} else {
			/*
			* non sono nella condizione di update
			* -> inserisco direttamente
			*/
			DBRes = DBExecQuery(bTrace, szInsertSQLCmd);
			if(DBresultStatus(DBRes)==DBRES_FATAL_ERROR){
				nResult=DBRES_FATAL_ERROR;
			}
			DBclear(DBRes);
		}

		if (bUpdate){
			/* faccio pulizia */
			for(nIndex=0;nIndex<nKeyFieldsNumber;nIndex++){
				free(pszKeyFieldsData[nIndex]);
			}
		}
		return nResult;
	} else {
		nResult=FALSE;
	}
	return nResult;
}



/*
* ep_bool_t ReadTableInfo(PDBSTRUCT pDB,char *szCfgBaseName)
* Lettura delle informazioni relative ad una tabella 
*/
ep_bool_t ReadTableInfo(PDBSTRUCT pDB,char *szPath,char *szCfgBaseName)
{
	char szCfgFile[128];

	if(pDB==NULL){
		return FALSE;
	}
	/*
	* creo i nomi di: Archivio, Indice, Cfg file
	*/
	sprintf(szCfgFile,"%s/%s.cfg",szPath,szCfgBaseName);

	pDB->nTagsNumber=ReadTagsInfo(szCfgFile,pDB);

	if((ReadFieldsInfo(szCfgFile,pDB))==-1){
		return(FALSE);
	} else {
		return(TRUE);
	}
}


void InsertTableInfo(PDBSTRUCT pDB,char *szCfgPath,char *szCfgName)
{
	DB_ARRAY_ITEM_STRUCT DBA;

	if(tTablesArray==NULL){
		tTablesArray=g_array_new(TRUE,TRUE,sizeof(DB_ARRAY_ITEM_STRUCT));
	}
	DBA.pDB=pDB;
	strcpy(DBA.szCfgPath,szCfgPath);
	strcpy(DBA.szCfgName,szCfgName);

	g_array_append_val(tTablesArray,DBA);
}

void FreeTablesInfo(void)
{
	if(tTablesArray) g_array_free(tTablesArray,TRUE);
}

/*
* lettura configurazione tabelle
*/
void ReadTablesInfo(void)
{
	int nIndex=0;
	PDB_ARRAY_ITEM_STRUCT pDBA;

	while((pDBA=&g_array_index(tTablesArray,DB_ARRAY_ITEM_STRUCT,nIndex))){

		if(pDBA->pDB==NULL){
			break;
		}
		if(ReadTableInfo(pDBA->pDB,pDBA->szCfgPath,pDBA->szCfgName)==FALSE){
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Errore in Apertura / Creazione [%s/%s]",pDBA->szCfgPath,pDBA->szCfgName);
#endif
		}

		nIndex++;
	}
}

/*
* ReadFieldsInfo()
*
* Fields structure inizialization
* legge i dati contenuti nel file di inizializzazione e
* riempie la struttura preposta alla loro memorizzazione
*
* ritorna:
* >=0 : numero di campi allocati
*  -1 : errore in allocazione campi
*/
int ReadFieldsInfo(char *szCfgFile,PDBSTRUCT pDB)
{
	PEP_FIELD pField;
	int   nFieldIndex;
	char  szBuffer[256];
	char  szFieldNumber[80];
	char  szParagraph[80];
	char  *pPtr;
	char  *szSeparator=" ,\t\n";
	char  *szDescrSeparator=",\n";

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	/*
	* Nome Data Base
	*/
	GetFileString(szParagraph,"DataBaseName", "", pDB->szDBName,sizeof(pDB->szDBName),szCfgFile,NULL);

	/*
	* Nome Tabella
	*/
	GetFileString(szParagraph,"TableName", "", pDB->szTableName,sizeof(pDB->szTableName),szCfgFile,NULL);

	pDB->nFieldsNumber   = GetFileInt(szParagraph,"FieldsNumber", 0,szCfgFile,NULL);
	pDB->nRecordLen      = GetFileInt(szParagraph,"RecordLen",    0,szCfgFile,NULL);
	pDB->nRecordTypePos  = GetFileInt(szParagraph,"RecordTypePos",1,szCfgFile,NULL);
	pDB->nRecordTypeLen  = GetFileInt(szParagraph,"RecordTypeLen",0,szCfgFile,NULL);
	pDB->nLineNumberPos  = GetFileInt(szParagraph,"LineNumberPos",1,szCfgFile,NULL);
	pDB->nDataStart      = GetFileInt(szParagraph,"DataStart",    1,szCfgFile,NULL);
	pDB->nLinesNumber    = GetFileInt(szParagraph,"LinesNumber",  0,szCfgFile,NULL);
	pDB->nTableMask      = GetFileInt(szParagraph,"TableMask",    INSERT_MASK | DELETE_MASK | CHANGE_MASK,szCfgFile,NULL);

	GetFileString(szParagraph,"RecordType","",pDB->szRecordType,sizeof(pDB->szRecordType),szCfgFile,NULL);

	/*
	* normalizzazione dati (offset 0)
	*/
	pDB->nRecordTypePos = max(0,pDB->nRecordTypePos-1);
	pDB->nLineNumberPos = max(0,pDB->nLineNumberPos-1);
	pDB->nDataStart     = max(0,pDB->nDataStart-1);

	nFieldIndex=0;
	strcpy(szParagraph,"Fields");
	while(nFieldIndex<pDB->nFieldsNumber){
		sprintf(szFieldNumber,"Field_%02d",nFieldIndex);
		GetFileString(szParagraph,szFieldNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
		if(strlen(szBuffer)){
			pDB->pFields[nFieldIndex]=(PEP_FIELD)malloc(sizeof(EP_FIELD));
			memset(pDB->pFields[nFieldIndex],0,sizeof(EP_FIELD));
			pField=pDB->pFields[nFieldIndex];
			/* 
			* Field Name 
			*/
			if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
				strcpy(pField->szFieldName,pPtr);
			} else {return(-1);}
			/* 
			* Field Type
			*/
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pField->cFieldType=*pPtr;
			} else {return(-1);}
			/*
			* Field Len
			*/
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pField->nFieldLen=atoi(pPtr);
			} else {return(-1);}
			/*
			* Field Dec
			*/
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pField->nFieldDec=atoi(pPtr);
			} else {return(-1);}
			/*
			* rm 15-01-1999 : Dati per multilinea solo se LinesNumber>0
			*/
			if(pDB->nLinesNumber){
				/*
				* Line Number : Numero di riga se record multilinea
				*/
				if((pPtr=strtok(NULL,szSeparator))!=NULL){
					pField->nLineNumber=atoi(pPtr);
				} else {return(-1);}
				/*
				* Field Offset : offset del campo nel file ascii (offset 1)
				*/
				if((pPtr=strtok(NULL,szSeparator))!=NULL){
					pField->nFieldOffset=atoi(pPtr);
				} else {return(-1);}
				/*
				* normalizzazione dati (offset 0)
				*/
				pField->nFieldOffset -=1;
			}
			/* Field Flag  */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pField->nFieldFlag=atoi(pPtr);
			} else {return(-1);}
			/*
			* Field Title
			*/
			if((pPtr=strtok(NULL,szDescrSeparator))!=NULL){
				strncpy(pField->szFieldTitle,pPtr,FIELD_TITLE_LEN-1);
			} else {return(-1);}
			/*
			* Field Description
			*/
			if((pPtr=strtok(NULL,szDescrSeparator))!=NULL){
				strncpy(pField->szFieldDescr,pPtr,FIELD_DESCR_LEN-1);
			} else {return(-1);}
		}
		nFieldIndex++;
	}

	return nFieldIndex;
}

/*
* ReadTagsInfo()
* alloca e legge i dati relativi ai tag per gli indici
*/
int ReadTagsInfo(char *szCfgFile,PDBSTRUCT pDB)
{
	int   nTagIndex;
	int   nTagsNumber;
	char  szBuffer[128];
	char  szTagNumber[80];
	char  szParagraph[80];
	char  *pPtr;
	char  *szSeparator=" ,\n";
	char  *szExprSeparator="\n";
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");
	nTagsNumber=GetFileInt(szParagraph,"TagsNumber",0,szCfgFile,NULL);

	nTagIndex=0;
	strcpy(szParagraph,"Tags");
	while(nTagIndex<nTagsNumber){
		sprintf(szTagNumber,"Tag_%02d",nTagIndex);
		GetFileString(szParagraph,szTagNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
		if(strlen(szBuffer)){
			/* Nome tag */
			if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
				strcpy(pDB->Tags[nTagIndex].szTagName,pPtr);
			} else {return(-1);}
			/* Unique */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pDB->Tags[nTagIndex].nUnique=atoi(pPtr);
			} else {return(-1);}
			/* Expression */
			if((pPtr=strtok(NULL,szExprSeparator))!=NULL){
				strcpy(pDB->Tags[nTagIndex].szExpression,pPtr);
			} else {return(-1);}
		}
		nTagIndex++;
	}

	return nTagsNumber;
}

#ifdef USE_POSTGRES
#include "ep_pgsql.c"
#endif

DBExecStatusType DBresultStatus(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	return PQresultStatus((PGresult *)DBRes);
#endif
}

char *DBcmdTuples(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	return (char *)PQcmdTuples((PGresult *)DBRes);
#endif
}

int DBnfields(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	return PQnfields((PGresult *)DBRes);
#endif
}

int DBntuples(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	return PQntuples((PGresult *)DBRes);
#endif
}

unsigned int DBgetlength(DBresult *DBRes,int nTuple,int nField)
{
#ifdef USE_POSTGRES
	return PQgetlength((PGresult *)DBRes,nTuple,nField);
#endif
}

char *DBfname(DBresult *DBRes,int nField)
{
#ifdef USE_POSTGRES
	return PQfname((PGresult *)DBRes,nField);
#endif
}

int DBftype(DBresult *DBRes,int nField)
{
#ifdef USE_POSTGRES
	return PQftype((PGresult *)DBRes,nField);
#endif
}

int DBfnumber(DBresult *DBRes,char *szFieldName)
{
#ifdef USE_POSTGRES
	return PQfnumber((PGresult *)DBRes,szFieldName);
#endif
}

char *DBgetvalue(DBresult *DBRes,int nTuple,int nField)
{
#ifdef USE_POSTGRES
	return PQgetvalue((PGresult *)DBRes,nTuple,nField);
#endif
}

char *DBresultErrorMessage(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	return (char *)PQresultErrorMessage((DBresult *)DBRes);
#endif
}
static ep_bool_t ConnectionActive=FALSE;

ep_bool_t DBTestConnection(ep_bool_t bReset)
{
	ep_bool_t bOK=TRUE;

	if(CONNECTION_OK!=PQstatus(DB)){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Connection to database is BAD trying to reconnect");
#endif
		bOK=FALSE;
		if(bReset){
			PQreset(DB);
		}
	}
	return bOK;
}

ep_bool_t DBConnect(char *szHost,char *szPort,char *szDataBase)
{
	ep_bool_t bRetValue=FALSE;

	if(ConnectionActive){
		return FALSE;
	}

#ifdef USE_POSTGRES
	if((DB=pgsql_connect_db(szHost,szPort,szDataBase))!=(DBconn *)NULL){
		bRetValue=TRUE;
	}
#endif

	ConnectionActive=bRetValue;;

	return bRetValue;
}

ep_bool_t DBReconnectUser(char *szHost,char *szPort,char *szDataBase,char *szUserName,char *szPassword)
{
	ep_bool_t bRetValue=FALSE;

	if(ConnectionActive){
		bRetValue=DBDisconnect();
	}

	bRetValue=DBConnectUser(szHost,szPort,szDataBase,szUserName,szPassword);

	ConnectionActive=bRetValue;;

	return bRetValue;
}

ep_bool_t DBConnectUser(char *szHost,char *szPort,char *szDataBase,char *szUserName,char *szPassword)
{
	ep_bool_t bRetValue=FALSE;

	if(ConnectionActive){
		return FALSE;
	}

#ifdef USE_POSTGRES
	if((DB=pgsql_connect_db_user(szHost,szPort,szDataBase,szUserName,szPassword))!=(DBconn *)NULL){
		bRetValue=TRUE;
	}
#endif

	ConnectionActive=bRetValue;;

	return bRetValue;
}

ep_bool_t DBDisconnect(void)
{
	if(!ConnectionActive){
		return FALSE;
	}
#ifdef USE_POSTGRES
	pgsql_disconnect_db(DB);
#endif
	ConnectionActive=FALSE;
	return TRUE;
}

ep_bool_t DBConnectionActive(void)
{
	return ConnectionActive;
}

DBresult *DBExecQuery(ep_bool_t bTrace,char *format, ...)
{
	va_list ap;
	char szQuery[4096];
	DBresult *DBRes=(DBresult *)NULL;

	if(DB==NULL) return NULL;

	va_start (ap, format);
	vsprintf (szQuery, format, ap);
	va_end (ap);
#ifdef TRACE
	if(bTrace) trace_debug(TRUE, TRUE, "EXEC SQL [%s]", szQuery);
#endif

#ifdef USE_POSTGRES
	DBRes=pgsql_exec(DB,szQuery);
#endif

#ifdef TRACE
	/*
	* Controllo ritorno del comando SQL
	* visualizzo comunque l'eventuale errore anche se bTrace e' FALSE
	*/
	switch(DBresultStatus(DBRes)){
		case DBRES_COMMAND_OK:
			if(bTrace)trace_debug(TRUE, TRUE, "DBRES_COMMAND_OK : [%5d] tuples",atoi(DBcmdTuples(DBRes)));
		break;
		case DBRES_EMPTY_QUERY:
			trace_debug(TRUE, TRUE, "DBRES_EMPTY_QUERY : %s",DBresultErrorMessage(DBRes));
		break;
		case DBRES_TUPLES_OK:
			if(bTrace)trace_debug(TRUE, TRUE, "DBRES_TUPLES_OK : [%5d] tuples", DBntuples (DBRes));
		break;
		case DBRES_COPY_OUT:
			if(bTrace)trace_debug(TRUE, TRUE, "DBRES_COPY_OUT");
		break;
		case DBRES_COPY_IN:
			if(bTrace)trace_debug(TRUE, TRUE, "DBRES_COPY_IN");
		break;
		case DBRES_BAD_RESPONSE:
			trace_debug(TRUE, TRUE, "DBRES_BAD_RESPONSE : %s",DBresultErrorMessage(DBRes));
		break;
		case DBRES_NONFATAL_ERROR:
			trace_debug(TRUE, TRUE, "DBRES_NONFATAL_ERROR : %s\n -> %s",DBresultErrorMessage(DBRes),szQuery);
		break;
		case DBRES_FATAL_ERROR:
			trace_debug(TRUE, TRUE, "DBRES_FATAL_ERROR : %s\n -> %s",DBresultErrorMessage(DBRes),szQuery);
		break;
	}
#endif

	return DBRes;
}

void DBclear(DBresult *DBRes)
{
#ifdef USE_POSTGRES
	pgsql_free_result(DBRes);
#endif
}

static DbType db_types[] = {
	{  16,  "bool",           DB_TYPE_STRING},
	{  17,  "bytea",          DB_TYPE_STRING},
	{  18,  "char",           DB_TYPE_STRING},
	{  19,  "name",           DB_TYPE_STRING},
	{  20,  "int8",           DB_TYPE_INTEGER},
	{  21,  "int2",           DB_TYPE_INTEGER},
	{  22,  "int2vector",     DB_TYPE_STRING},
	{  23,  "int4",           DB_TYPE_INTEGER},
	{  24,  "regproc",        DB_TYPE_STRING},
	{  25,  "text",           DB_TYPE_STRING},
	{  26,  "oid",            DB_TYPE_STRING},
	{  27,  "tid",            DB_TYPE_STRING},
	{  28,  "xid",            DB_TYPE_STRING},
	{  29,  "cid",            DB_TYPE_STRING},
	{  30,  "oidvector",      DB_TYPE_STRING},
	{  32,  "SET",            DB_TYPE_STRING},
	{  71,  "pg_type",        DB_TYPE_STRING},
	{  75,  "pg_attribute",   DB_TYPE_STRING},
	{  81,  "pg_proc",        DB_TYPE_STRING},
	{  83,  "pg_class",       DB_TYPE_STRING},
	{  86,  "pg_shadow",      DB_TYPE_STRING},
	{  87,  "pg_group",       DB_TYPE_STRING},
	{  88,  "pg_database",    DB_TYPE_STRING},
	{ 210,  "smgr",           DB_TYPE_STRING},
	{ 600,  "point",          DB_TYPE_STRING},
	{ 601,  "lseg",           DB_TYPE_STRING},
	{ 602,  "path",           DB_TYPE_STRING},
	{ 603,  "box",            DB_TYPE_STRING},
	{ 604,  "polygon",        DB_TYPE_STRING},
	{ 628,  "line",           DB_TYPE_STRING},
	{ 629,  "_line",          DB_TYPE_STRING},
	{ 650,  "cidr",           DB_TYPE_STRING},
	{ 651,  "_cidr",          DB_TYPE_STRING},
	{ 700,  "float4",         DB_TYPE_FLOAT},
	{ 701,  "float8",         DB_TYPE_FLOAT},
	{ 702,  "abstime",        DB_TYPE_DATETIME},
	{ 703,  "reltime",        DB_TYPE_STRING},
	{ 704,  "tinterval",      DB_TYPE_STRING},
	{ 705,  "unknown",        DB_TYPE_STRING},
	{ 718,  "circle",         DB_TYPE_STRING},
	{ 719,  "_circle",        DB_TYPE_STRING},
	{ 790,  "money",          DB_TYPE_STRING},
	{ 791,  "_money",         DB_TYPE_STRING},
	{ 829,  "macaddr",        DB_TYPE_STRING},
	{ 869,  "inet",           DB_TYPE_STRING},
	{1000,  "_bool",          DB_TYPE_STRING},
	{1001,  "_bytea",         DB_TYPE_STRING},
	{1002,  "_char",          DB_TYPE_STRING},
	{1003,  "_name",          DB_TYPE_STRING},
	{1005,  "_int2",          DB_TYPE_INTEGER},
	{1006,  "_int2vector",    DB_TYPE_STRING},
	{1007,  "_int4",          DB_TYPE_INTEGER},
	{1008,  "_regproc",       DB_TYPE_STRING},
	{1009,  "_text",          DB_TYPE_STRING},
	{1010,  "_tid",           DB_TYPE_STRING},
	{1011,  "_xid",           DB_TYPE_STRING},
	{1012,  "_cid",           DB_TYPE_STRING},
	{1013,  "_oidvector",     DB_TYPE_STRING},
	{1014,  "_bpchar",        DB_TYPE_STRING},
	{1015,  "_varchar",       DB_TYPE_STRING},
	{1016,  "_int8",          DB_TYPE_INTEGER},
	{1017,  "_point",         DB_TYPE_STRING},
	{1018,  "_lseg",          DB_TYPE_STRING},
	{1019,  "_path",          DB_TYPE_STRING},
	{1020,  "_box",           DB_TYPE_STRING},
	{1021,  "_float4",        DB_TYPE_FLOAT},
	{1022,  "_float8",        DB_TYPE_FLOAT},
	{1023,  "_abstime",       DB_TYPE_STRING},
	{1024,  "_reltime",       DB_TYPE_STRING},
	{1025,  "_tinterval",     DB_TYPE_STRING},
	{1027,  "_polygon",       DB_TYPE_STRING},
	{1028,  "_oid",           DB_TYPE_STRING},
	{1033,  "aclitem",        DB_TYPE_STRING},
	{1034,  "_aclitem",       DB_TYPE_STRING},
	{1040,  "_macaddr",       DB_TYPE_STRING},
	{1041,  "_inet",          DB_TYPE_STRING},
	{1042,  "bpchar",         DB_TYPE_STRING},
	{1043,  "varchar",        DB_TYPE_STRING},
	{1082,  "date",           DB_TYPE_DATE},
	{1083,  "time",           DB_TYPE_TIME},
	{1114,  "timestamp",      DB_TYPE_DATETIME},
	{1115,  "_timestamp",     DB_TYPE_DATETIME},
	{1182,  "_date",          DB_TYPE_DATE},
	{1183,  "_time",          DB_TYPE_TIME},
	{1184,  "timestamptz",    DB_TYPE_DATETIME},
	{1185,  "_timestamptz",   DB_TYPE_DATETIME},
	{1186,  "interval",       DB_TYPE_STRING},
	{1187,  "_interval",      DB_TYPE_STRING},
	{1231,  "_INTEGER",       DB_TYPE_INTEGER},
	{1266,  "timetz",         DB_TYPE_TIME},
	{1270,  "_timetz",        DB_TYPE_TIME},
	{1560,  "bit",            DB_TYPE_STRING},
	{1561,  "_bit",           DB_TYPE_STRING},
	{1562,  "varbit",         DB_TYPE_STRING},
	{1563,  "_varbit",        DB_TYPE_STRING},
	{1700,  "numeric",        DB_TYPE_INTEGER},
	{1790,  "refcursor",      DB_TYPE_STRING},
	{0,NULL,0}
};

DBFieldType db_get_field_type(int nType)
{
	int i=0;

	while(db_types[i].type_name){
		if(db_types[i].type_id == nType){
			return db_types[i].type;
		}
		i++;
	}

	return DB_TYPE_NONE;
}

/*
* PrintListFromSelect()
* ritorna il numero di tuple o -1 se errore
*/
int PrintListFromSelect(FILE *fp,char *szSqlCmd, char *szOrdProg, int nNumCollo)
{
	int nTuples;
	int nFields;
	int nIndex;
	DBFieldType nType;
	char szBuffer[128];
	char szString[256];
	int nFieldIndex;
	DBresult *DBRes = NULL;
	int nFieldLength[256];

	/*
	* Eseguo la select specificata
	*/
	DBRes=DBExecQuery(0,szSqlCmd);

	nTuples=DBntuples(DBRes);
	nFields=DBnfields(DBRes);

	if(nTuples){
		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* ricavo le lunghezze dei titoli
			*/ 
			nType=db_get_field_type(DBftype(DBRes,nFieldIndex));
			switch(nType){
				default:
				case DB_TYPE_STRING:
					nFieldLength[nFieldIndex]=max(DBgetlength(DBRes,0,nFieldIndex),strlen(DBfname(DBRes,nFieldIndex)));
				break;
				case DB_TYPE_FLOAT:
				case DB_TYPE_INTEGER:
					nFieldLength[nFieldIndex]=strlen(DBfname(DBRes,nFieldIndex));
				break;
			}
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer);
			fprintf(fp,"+");
		}
		fprintf(fp,"\n");

		fprintf(fp,"|");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* titolo colonna 
			*/ 
			fprintf(fp,"%-*.*s|",nFieldLength[nFieldIndex],nFieldLength[nFieldIndex],DBfname(DBRes,nFieldIndex));
		}
		fprintf(fp,"\n");

		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer); fprintf(fp,"+");
		}
		fprintf(fp,"\n");

		/*
		* Aggiungo le tuple alla lista (una alla volta)
		*/
		for(nIndex=0;nIndex<nTuples;nIndex++){

			fprintf(fp,"|");
			for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				nType=db_get_field_type(DBftype(DBRes,nFieldIndex));
				switch(nType){
					case DB_TYPE_STRING:
						/*
						* rm 22-05-2003  : normalizzo la stringa per la stampa
						* elimino caratteri sopra il 128
						*/
						strcpy(szString,DBgetvalue(DBRes,nIndex,nFieldIndex));
						StrNormalize(szString);

						fprintf(fp,"%*s|",nFieldLength[nFieldIndex],szString);
					break;
					case DB_TYPE_FLOAT:
						fprintf(fp,"%*.*f|",nFieldLength[nFieldIndex],3,strtod(DBgetvalue(DBRes,nIndex,nFieldIndex),NULL));
					break;
					case DB_TYPE_INTEGER:
						fprintf(fp,"%*d|",nFieldLength[nFieldIndex],atoi(DBgetvalue(DBRes,nIndex,nFieldIndex)));
					break;
					default:
						fprintf(fp,"%*s|",nFieldLength[nFieldIndex],DBgetvalue(DBRes,nIndex,nFieldIndex));
					break;
				}
			}
			fprintf(fp,"\n");
		}
		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer); fprintf(fp,"+");
		}
		fprintf(fp,"\n");

	}
	
	DBclear(DBRes);

	return nTuples;
}

ep_bool_t PrintTable(char *szCfgFileName,char *szParagraph,char *szTableName,char *szPrinterName,char *szPathStampe,GList *PSList,FILE *fp,int nPrintType)
{
	char szSelectCmd[2048];
	char szBuffer[256];
	char szDateBuffer[256];
	char szTimeBuffer[256];
	char szTitle[256];
	FILE *fpFile;
	char szFileName[128];
	int nRC;
	ep_bool_t bOK=TRUE;

	if(strlen(szParagraph) && strlen(szTableName)){
		strcpy(szBuffer,szTableName);
		strcat(szBuffer,"_descr");
		GetFileString(szParagraph,szBuffer, "", szTitle, sizeof(szTitle),szCfgFileName,NULL); 

		if(fp==(FILE *)NULL){
			sprintf(szFileName,"%s/%s",szPathStampe,szTableName);
			if ((fpFile=fopen(szFileName,"w"))==(FILE *)NULL) {
				bOK=FALSE;
			}
			if (bOK) fprintf(fpFile,"A&L Easy Picking - %s %s\n%s\n\n",GetDate(szDateBuffer), GetTime(szTimeBuffer),szTitle);
		} else {
			fpFile=fp;
		}


		GetFileString(szParagraph,szTableName, "", szSelectCmd, sizeof(szSelectCmd),szCfgFileName,NULL); 

		if(PSList){
			ParseStatement(szSelectCmd,PSList);
		}

		nRC=PrintListFromSelect(fpFile,szSelectCmd,0,0);

		/*
		* Salto pagina
		*/
		if(fp==(FILE *)NULL){
			fprintf(fpFile,"");
			fclose(fpFile);

			if(nRC){
				bOK=PrintFile(szFileName,szPrinterName,szTitle,nPrintType,FALSE);
			} else {
				bOK=FALSE;
			}
		}
	} else {
		bOK=FALSE;
	}
	return bOK;
}

ep_bool_t WriteTable(char *szCfgFileName,char *szParagraph,char *szTableName,char *szFileName,GList *PSList)
{
	char szSelectCmd[2048];
	FILE *fpFile;
	ep_bool_t bOK=TRUE;

	if(strlen(szParagraph) && strlen(szTableName)){

		if ((fpFile=fopen(szFileName,"w"))) {

			GetFileString(szParagraph,szTableName, "", szSelectCmd, sizeof(szSelectCmd),szCfgFileName,NULL); 

			if(PSList){
				ParseStatement(szSelectCmd,PSList);
			}

			bOK=PrintListFromSelect(fpFile,szSelectCmd,0,0);

			fclose(fpFile);
		} else {
			bOK=FALSE;
		}

	} else {
		bOK=FALSE;
	}
	return bOK;
}

