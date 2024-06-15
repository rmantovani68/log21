/*
* modulo : dbfun.c
* -----------------
* Libreria di gestione tabelle e configurazioni
*
* Progetto Picking - DB Management - PostgreSQL
*
* Data creazione 13/12/1995
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1995-98
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 07-07-1998 : Aggiunta compilazione condizionale per CodeBase 4.5/5.1
* rm 18-01-1999 : Aggiunta la gestione multi linea per DeAgostini
* rm 16-06-2000 : Eliminati i riferimenti a CodeBase - si usa PGSQL
* -------------   -------------------------------------------------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <libpq-fe.h>

#if defined(TRACE) || defined(TRACE_SQL)
	#include <trace.h>
#endif

#include "dbfun.h"
#include "ca_file.h"

static PGconn *DBConn;
static int ReadFieldsInfo(char *szCfgFile,PDBSTRUCT pDB);
static int ReadTagsInfo(char *szCfgFile,PDBSTRUCT pDB);
static char *DeleteNewLine(char *str);

static GArray *tArray=NULL;


/*
* Normalizza il fastidiosissimo apice (per PGSQL !)
*/
char *NormalizeString(char *szString)
{
	static char szDestination[1024];
	char szTmpString[1024];
	char *pPtr;
	int nIndex;

	/*
	* maschero i caratteri sopra il 127 ascii (0x7f)
	*/
	strcpy(szDestination,szString);
	nIndex=0;
	while (szDestination[nIndex]){
		if(!isascii(szDestination[nIndex])){
			szDestination[nIndex]='.';
		}
		nIndex++;
	}


	

	/*
	* sostituzione del (') 
	*/ 
	pPtr=szDestination;
	while ((pPtr=strchr(pPtr,'\''))!=(char *)NULL){
		/*
		* Tronco la stringa sull'apice
		*/
		*pPtr = '\0';
		strcpy(szTmpString,szDestination);
//		strcat(szTmpString,"\\'");
		/* 04-01-2012 : modifiche per Delta su Fedora 13 */
		strcat(szTmpString,"''");
		strcat(szTmpString,(pPtr+1));

		/*
		* Riporto la modifica
		*/
		strcpy(szDestination,szTmpString);
		pPtr+=2;
	}
	return (szDestination);
}

BOOL TestDataBaseConnection(BOOL bReset)
{
	BOOL bOK=TRUE;

	if(CONNECTION_OK!=PQstatus(DBConn)){
#ifdef TRACE
		trace_out_vstr_date(1, "Connection to database is BAD trying to reconnect");
#endif
		bOK=FALSE;
		if(bReset){
			PQreset(DBConn);
		}
	}
	return bOK;
}

BOOL ConnectDataBase(char *szHost,char *szPort,char *szDataBase)
{
	BOOL bRetValue;
	/*
	* connessione al database
	*/
	/*
	*pDBConn = PQsetdb(Cfg.szPGHost, Cfg.szPGPort, NULL, NULL, Cfg.szPGDataBase);
	*/
	DBConn = PQsetdb(szHost, szPort, NULL, NULL, szDataBase);
	
	/*
	* controllo correttezza della connessione
	*/
	if (PQstatus(DBConn) == CONNECTION_BAD) {
#ifdef TRACE
		trace_out_vstr_date(1, "Connection to database '%s' failed.", szDataBase);
		trace_out_vstr_date(1, "Error : %s", PQerrorMessage(DBConn));
#endif
		bRetValue=FALSE;
	} else {
#ifdef TRACE
		trace_out_vstr_date(1, "Connection to database '%s' OK.", szDataBase);
#endif
		bRetValue=TRUE;
	}
	return bRetValue;
}

void DisconnectDataBase(void)
{
	PQfinish(DBConn);
}


/*
* PGresult *PGExecSQL (BOOL bTrace, char *format, ...)
* Wrapper function for SQL statements
*/
PGresult *PGExecSQL (BOOL bTrace,char *format, ...)
{
	va_list ap;
	char command[4096];
	PGresult *retval;

	va_start (ap, format);
	vsprintf (command, format, ap);
	va_end (ap);
	if(bTrace){
#ifdef TRACE
		trace_out_vstr_date(1,"EXEC SQL [%s]", command);
#endif
	}
	retval = PQexec (DBConn, command);

#ifdef TRACE
	/*
	* Controllo ritorno del comando SQL
	* visualizzo comunque l'eventuale errore anche se bTrace e' FALSE
	*/
	switch(PQresultStatus(retval)){
		case PGRES_EMPTY_QUERY:
			trace_out_vstr_date(1,"PGRES_EMPTY_QUERY : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_COMMAND_OK:
			if(bTrace)trace_out_vstr_date(1,"PGRES_COMMAND_OK : [%5d] tuples",atoi(PQcmdTuples(retval)));
		break;
		case PGRES_TUPLES_OK:
			if(bTrace)trace_out_vstr_date(1,"PGRES_TUPLES_OK : [%5d] tuples", PQntuples (retval));
		break;
		case PGRES_COPY_OUT:
			if(bTrace)trace_out_vstr_date(1,"PGRES_COPY_OUT");
		break;
		case PGRES_COPY_IN:
			if(bTrace)trace_out_vstr_date(1,"PGRES_COPY_IN");
		break;
		case PGRES_BAD_RESPONSE:
			trace_out_vstr_date(1,"PGRES_BAD_RESPONSE : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_NONFATAL_ERROR:
			trace_out_vstr_date(1,"PGRES_NONFATAL_ERROR : %s",PQresultErrorMessage(retval));
		break;
		case PGRES_FATAL_ERROR:
			trace_out_vstr_date(1,"PGRES_FATAL_ERROR : %s",PQresultErrorMessage(retval));
		break;
	}
#endif

	return retval;
}




/*
* BOOL GetAsciiRecord(szBuffer,pDB)
* Legge dal buffer i dati da inserire nella tabella 
* rm + st : 16-06-2000 : portata su PGSQL
*/
BOOL GetAsciiRecord(char *szBuffer,PDBSTRUCT pDB)
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
	PGresult *PGRes;

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
		PGRes=PGExecSQL(FALSE,szSQLCmd);
		PQclear(PGRes);
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
BOOL WriteAsciiRecord(FILE *fp,PDBSTRUCT pDB,PGresult *PGRes,int nIndex,BOOL bFillZero,BOOL bSigned)
{
	char szRecord[4096];
	char szBuffer[2048];
	PFIELD pField;
	int nFieldIndex;
	int nFieldPosition;
	int nFieldLen;
	int nRecordLen=0;
	int nFieldsNumber=0;

	nFieldsNumber=PQnfields(PGRes);

	/*
	* Ricavo la lunghezza del buffer di scrittura
	*/

#ifdef RECORD_LEN /* modifica per Mondadori */
	
	nRecordLen=0;
	for(nFieldIndex=0;nFieldIndex<nFieldsNumber;nFieldIndex++){
		if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
			nRecordLen+=pDB->pFields[nFieldIndex]->nFieldLen;
		}
	}
	nRecordLen+=pDB->nRecordTypeLen;
#endif

	nRecordLen=pDB->nRecordLen;

	/* se recordlen non presente lo calcolo */
	if(nRecordLen==0){
		nRecordLen=0;
		for(nFieldIndex=0;nFieldIndex<nFieldsNumber;nFieldIndex++){
			if(pDB->pFields[nFieldIndex]->nFieldFlag & IMPORT_MASK){
				nRecordLen+=pDB->pFields[nFieldIndex]->nFieldLen;
			}
		}
		nRecordLen+=pDB->nRecordTypeLen;
	}




	memset(szRecord,' ',nRecordLen+1);
	szRecord[nRecordLen-1] = '*';
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
		if(pField->nFieldFlag & IMPORT_MASK){
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
					sprintf(szBuffer, "%-*s", nFieldLen,PQgetvalue(PGRes,nIndex,nFieldIndex));
				break;
				case 'N':
				case 'n':
					if(bFillZero){
						sprintf(szBuffer, "%0*d", nFieldLen,atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
					} else {
						sprintf(szBuffer, "%*d", nFieldLen,atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
					}
					if(bSigned){
						szBuffer[0]=atoi(PQgetvalue(PGRes,nIndex,nFieldIndex))>=0?'+':'-';
					}
				break;
			}
			memcpy(szRecord+nFieldPosition, szBuffer, nFieldLen);
			nFieldPosition+=nFieldLen;
		}
	}
	fprintf(fp,szRecord);

	return(TRUE);
}


/*
* IsAsciiRecord(szBuffer,pDB)
*/
BOOL IsAsciiRecord(char *szBuffer,PDBSTRUCT pDB)
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
* BOOL UpdateAsciiRecord(char *szBuffer,PDBSTRUCT pDB, BOOL bUpdate)
* + st  02/10/2000 
* Legge i dati da szBuffer:
* - se bUpdate=TRUE effettua un ricerca nella tabella utilizzando la chiave "primaria -> UNIQUE e COSTITUITA DA UN SOLO CAMPO" 
*              (la prima indicata nel file di configurazione .cfg) effettuando un update se trova gia' presente un riga; altrimenti inserisce;
* - se bUpdate=FALSE inserisce direttamente in tabella (ATTENZIONE ALLE CHIAVI "primarie");
* + rm  08/08/2001 
* gestione indice composto da piu' campi
*/
BOOL UpdateAsciiRecord(char *szBuffer, PDBSTRUCT pDB, BOOL bUpdate)
{
	int nFieldPosition;
	int nFieldIndex;
	int nIndex;
	int nLen;
	char szFieldString[128];
	char szUpdateSQLCmd[1024];
	char szInsertSQLCmd[1024];
	char *pszKeyFieldsData[128];
	int nKeyFields[128];
	int nKeyFieldsNumber=0;
	char *pszBuffer;
	char szTmpBuffer[128];
	char *pPtr;
	char  *szSeparator="+";
	PGresult *PGRes;


	DeleteNewLine(szBuffer);
	nLen=strlen(szBuffer);

	/*
	* Verifico corrispondenza tipo record
	*/
	if(nLen >= pDB->nRecordLen && !strncmp(szBuffer+pDB->nRecordTypePos,pDB->szRecordType,pDB->nRecordTypeLen)){
		/*
		* inserimento dei dati in tabelle PostgreSQL
		*/
		/*
		* Se Update allora genero la lista dei campi chiave
		*/
		if(bUpdate){
			if(pDB->Tags==(PTAG)NULL){
				fprintf(stderr,"Errore : Overwrite e Indice [0] non presente !!!\n");
#ifdef TRACE
				trace_out_vstr_date(1,"UpdateAsciiRecord(%s) : Errore : Overwrite e Indice [0] non presente !!!" , pDB->szTableName);
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
				trace_out_vstr_date(1,"UpdateAsciiRecord(%s) : Errore : Overwrite attivo e campo indice non presente [%s]" , pDB->szTableName,pDB->Tags[0].szExpression);
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

		strcpy (szInsertSQLCmd,"INSERT INTO " );
		strcat (szInsertSQLCmd, pDB->szTableName);
		strcat (szInsertSQLCmd," VALUES ( "); 

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


				/*
				* Conversione per il valori numerici
				*/
				if(pDB->pFields[nFieldIndex]->cFieldType=='N'){
					if(pDB->pFields[nFieldIndex]->nFieldDec){
						sprintf(szFieldString,"%0f",atof(szFieldString));
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
			* con bUpdate=TRUE -> se PQcmdTuples ritorna 1 
			* - la riga era gia'presente ed e' stata aggiornata;
			* - altrimenti significa che la riga non e' presente e va inserita
			*/
#ifdef TRACE_ELIMINATO
			trace_out_vstr_date (1, "PQexec : %s",szUpdateSQLCmd);
#endif
			PGRes =  PQexec(DBConn, szUpdateSQLCmd);
			if (!PGRes || PQresultStatus(PGRes) != PGRES_COMMAND_OK ) {
				fprintf (stderr, "\nUPDATE TABLE %s: COMMAND FAILED\n",pDB->szTableName);
				fprintf (stderr, "SQL Expression : '%s'\n",szUpdateSQLCmd);
				fprintf (stderr, PQerrorMessage(DBConn));
#ifdef TRACE
				trace_out_vstr_date (1, "UPDATE TABLE %s: COMMAND FAILED",pDB->szTableName);
				trace_out_vstr_date (1, "SQL Expression : '%s'",szUpdateSQLCmd);
				trace_out_vstr_date (1, "SQL Error      : '%s'",PQerrorMessage(DBConn));
#endif
			}
			/*
			* Update fallito a causa di record non presente (lo inserisco)
			*/
			if (atoi(PQcmdTuples(PGRes))==0){
				PGRes = PQexec(DBConn, szInsertSQLCmd);
			}
			if (!PGRes || PQresultStatus(PGRes) != PGRES_COMMAND_OK) {
				fprintf (stderr, "\nINSERT IN TABLE %s: COMMAND FAILED\n",pDB->szTableName);
				fprintf (stderr, "SQL Expression : '%s'\n",szInsertSQLCmd);
				fprintf (stderr, PQerrorMessage(DBConn));
#ifdef TRACE
				trace_out_vstr_date (1, "INSERT IN TABLE %s: COMMAND FAILED",pDB->szTableName);
				trace_out_vstr_date (1, "SQL Expression : '%s'",szInsertSQLCmd);
				trace_out_vstr_date (1, "SQL Error      : '%s'",PQerrorMessage(DBConn));
#endif
			}
		} else {
			/*
			* non sono nella condizione di update
			* -> inserisco direttamente
			*/
			PGRes = PQexec(DBConn, szInsertSQLCmd);
			if (!PGRes || PQresultStatus(PGRes) != PGRES_COMMAND_OK) {
				fprintf (stderr, "\nINSERT IN TABLE %s: COMMAND FAILED\n",pDB->szTableName);
				fprintf (stderr, "SQL Expression : '%s'\n",szInsertSQLCmd);
				fprintf (stderr, PQerrorMessage(DBConn));
#ifdef TRACE
				trace_out_vstr_date (1, "INSERT IN TABLE %s: COMMAND FAILED",pDB->szTableName);
				trace_out_vstr_date (1, "SQL Expression : '%s'",szInsertSQLCmd);
				trace_out_vstr_date (1, "SQL Error      : '%s'",PQerrorMessage(DBConn));
#endif
			}
		}

		PQclear(PGRes);
		if (bUpdate){
			/* faccio pulizia */
			for(nIndex=0;nIndex<nKeyFieldsNumber;nIndex++){
				free(pszKeyFieldsData[nIndex]);
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}



/*
* BOOL ReadTableInfo(PDBSTRUCT pDB,char *szCfgBaseName)
* Lettura delle informazioni relative ad una tabella di PGSQL
*/
BOOL ReadTableInfo(PDBSTRUCT pDB,char *szPath,char *szCfgBaseName)
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

	if(tArray==NULL){
		tArray=g_array_new(TRUE,TRUE,sizeof(DB_ARRAY_ITEM_STRUCT));
	}
	DBA.pDB=pDB;
	strcpy(DBA.szCfgPath,szCfgPath);
	strcpy(DBA.szCfgName,szCfgName);

	g_array_append_val(tArray,DBA);
}

void FreeTablesInfo(void)
{
	g_array_free(tArray,TRUE);
}

/*
* lettura configurazione tabelle
*/
void ReadTablesInfo(void)
{
	int nIndex=0;
	PDB_ARRAY_ITEM_STRUCT pDBA;

	while((pDBA=&g_array_index(tArray,DB_ARRAY_ITEM_STRUCT,nIndex))){

		if(pDBA->pDB==NULL){
			break;
		}
		if(ReadTableInfo(pDBA->pDB,pDBA->szCfgPath,pDBA->szCfgName)==FALSE){
#ifdef TRACE
			trace_out_vstr_date(1, "Errore in Apertura / Creazione [%s/%s]",pDBA->szCfgPath,pDBA->szCfgName);
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
static int ReadFieldsInfo(char *szCfgFile,PDBSTRUCT pDB)
{
	PFIELD pField;
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
	xncGetFileString(szParagraph,"DataBaseName", "", pDB->szDBName,sizeof(pDB->szDBName),szCfgFile,NULL);

	/*
	* Nome Tabella
	*/
	xncGetFileString(szParagraph,"TableName", "", pDB->szTableName,sizeof(pDB->szTableName),szCfgFile,NULL);

	pDB->nFieldsNumber   = xncGetFileInt(szParagraph,"FieldsNumber", 0,szCfgFile,NULL);
	pDB->nRecordLen      = xncGetFileInt(szParagraph,"RecordLen",    0,szCfgFile,NULL);
	pDB->nRecordTypePos  = xncGetFileInt(szParagraph,"RecordTypePos",1,szCfgFile,NULL);
	pDB->nRecordTypeLen  = xncGetFileInt(szParagraph,"RecordTypeLen",0,szCfgFile,NULL);
	pDB->nLineNumberPos  = xncGetFileInt(szParagraph,"LineNumberPos",1,szCfgFile,NULL);
	pDB->nDataStart      = xncGetFileInt(szParagraph,"DataStart",    1,szCfgFile,NULL);
	pDB->nLinesNumber    = xncGetFileInt(szParagraph,"LinesNumber",  0,szCfgFile,NULL);
	pDB->nTableMask      = xncGetFileInt(szParagraph,"TableMask",    INSERT_MASK | DELETE_MASK | CHANGE_MASK,szCfgFile,NULL);

	xncGetFileString(szParagraph,"RecordType","",pDB->szRecordType,sizeof(pDB->szRecordType),szCfgFile,NULL);

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
		xncGetFileString(szParagraph,szFieldNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
		if(strlen(szBuffer)){
			pDB->pFields[nFieldIndex]=(PFIELD)malloc(sizeof(FIELD));
			memset(pDB->pFields[nFieldIndex],0,sizeof(FIELD));
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
static int ReadTagsInfo(char *szCfgFile,PDBSTRUCT pDB)
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
	nTagsNumber=xncGetFileInt(szParagraph,"TagsNumber",0,szCfgFile,NULL);

	nTagIndex=0;
	strcpy(szParagraph,"Tags");
	while(nTagIndex<nTagsNumber){
		sprintf(szTagNumber,"Tag_%02d",nTagIndex);
		xncGetFileString(szParagraph,szTagNumber,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL);
		if(strlen(szBuffer)){
			/* Nome tag */
			if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
				strcpy(pDB->Tags[nTagIndex].szTagName,pPtr);
			} else {return(-1);}
			/* Order */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				pDB->Tags[nTagIndex].nOrder=atoi(pPtr);
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

static char *DeleteNewLine(char *str)
{
	char *dst,*src;

	src=str+strlen(str)-1;
	while(*src=='\n' || *src=='\r') {
		*src='\0';
		src--;
	}
	*(src+1)='\0';
	dst=src=str;
	while((*dst++=*src++));
	return(str);
}
