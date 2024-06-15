/*
* stampa.c
*
* Funzioni di libreria per la gestione delle stampe.
*
* Autore: Daniele Ravaioli
*
* Copyright A&L srl 2003-2021
*/



/*
* Sarebbe opportuno aggiungere un evento di salto pagina
*/



#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termio.h>
#include <sys/stat.h>
#include <ctype.h>

#include <glib.h>
#include <ep-common.h>

#include "linklist.h"
#include "stampa.h"

#define PAGINA			"@@"

/*
* Strutture dati
*/
typedef struct tagDati{
	LINK_ITEM link;
  char *pszString;
  int nRow;
  int nColumn;
	ep_bool_t bIncrement;
	/*
	void *(UserFunc)(char *client);
	*/
} DATA,*PDATA;


/*
* Variabili globali al modulo
*/
LINKED_LIST FirstPageList;	/* Lista dati da stampare sulla prima pagina 	*/
LINKED_LIST LastPageList;		/* Lista dati da stampare sull'ultima pagina 	*/
LINKED_LIST EveryPageList;	/* Lista dati da stampare su ogni pagina 			*/
LINKED_LIST NewPageList;		/* Lista dati da stampare ad ogni cambio pagina sulla nuova pagina 		*/
LINKED_LIST OldPageList;		/* Lista dati da stampare ad ogni cambio pagina sulla vecchia pagina 	*/

int nRowsPage=0;		/* Numero delle righe della pagina (offset 1)		*/
int nColumnsPage=0;	/* Numero delle colonne della pagina (offset 1) */
int nPag=1;					/* Numero pagina (offset 1)											*/
int nCurrentRow=0;	/* Riga corrente (offset 1)											*/
int nFirstRow=0;		/* Prima riga dalla quale iniziare la stampa (offset 1)	*/
int nLastRow=0;			/* Ultima riga sulla quale stampare (offset 1)	*/

ep_bool_t bEnableNewPage=TRUE;		/* Stampa dati su nuova pagina abilitata/disabilitata 										*/
ep_bool_t bEnableOldPage=TRUE;		/* Stampa dati al cambio pagina su vecchia pagina abilitata/disabilitata 	*/
ep_bool_t bEnableEveryPage=TRUE;	/* Stampa dati al cambio pagina su nuova pagina abilitata/disabilitata 		*/
ep_bool_t bStato=FALSE;	/* E' TRUE se si e' riuscito ad aprire in scrittura il file di anteprima di stampa
											 i ad allocare la memoria per la pagina corrente. FALSE altrimenti */

char **StringVector;	/* Pagina in memoria */

FILE *FilePointer;	/* Puntatore al file che viene conerra' l'anteprima di stampa */


/*
******************************************************************************************
*
* Funzioni esportate
*
******************************************************************************************
*/



/*
* Descrizione generale:
* Questa funzione deve essere chiamata dopo la InitPrint() in quanto deve conoscere le dimen-
* sioni della pagina (righe e colonne).
* Inizializza le strutture dati per la stampa dei dati sulla prima pagina, ultima pagina, cambi
* pagina (vecchia e nuova pagina), e scritte che compaiono su tutte le pagine.
* La InitData() appende nuovi dati del tipo indicato da pszDataType alla lista relativa.
* Se la stringa fuoriesce dalla pagina essa viene troncata (non va a capo).
* 
* Descrizione parametri:
* -pszDataType deve contenere il tipo di dato che puo' essere:
*  # FirstPage: dati che vengono stampati solo sulla prima pagina
*  # LastPage : dati che vengono stampati solo sull'ultima pagina
*  # EveryPage: dati che vengono stampati in ognio pagina
*  # NewPage  : dati che vengono stampati in ogni nuova pagina 
*  # OldPage  : dati che vengono stampati su una pagina completa (prima di iniziare la nuova pagina)
* -pszString e' la stringa che si vuole stampare; il carattere speciale @@ viene sostituito con il
*						 numero di pagina
* -nRow e' il numero di riga alla quale si vuole stampare la stringa (offset 1)
* -nColumn e' il numero di colonna alla quale si vuole stampare la stringa (offset 1)
* -bIncrement e' significativo solo se DataType e' EveryPage col significato:
*	 	true a ogni pagina incrementa (incremento numerico) il campo pszString
*  	false non incrementa (campo fisso)
*
* Valore di ritorno:
*  il valore di ritorno e' un intero col seguente significato:
*   # 0 nessun errore rilevato
*   # 1 stampa non inizializzata con InitPrint
*   # 2 DataType non riconosciuto
*   # 3 nColumn >= al numero delle colonne della pagina o nRow > al numero delle righe della pagina
*   # 4 nRow o nColumn sono <=0
*/

int InitData(char *pszDataType,char *pszString,int nRow,int nColumn,ep_bool_t bIncrement)
{
	int nTipoDato=0;
	int nRetVal=0;

	 /*nPag=1;*/

	if (!bStato) nRetVal=1; 
	if (!nRetVal){
		/*
		* Controllo che nRow e nColumn siano all'interno della pagina e che siano positive.
		*/
		if (nColumn>=nColumnsPage || nRow>nRowsPage) nRetVal=3;
		if (nColumn<1 || nRow<1) nRetVal=4;
		if (!nRetVal){
			if (!strcmp(pszDataType,"FirstPage")) nTipoDato=1;
			if (!strcmp(pszDataType,"LastPage")) 	nTipoDato=2;
			if (!strcmp(pszDataType,"EveryPage")) nTipoDato=3;
			if (!strcmp(pszDataType,"NewPage")) 	nTipoDato=4;
			if (!strcmp(pszDataType,"OldPage")) 	nTipoDato=5;
			switch (nTipoDato){
				case 1: /* FirstPage */
					FillData(&FirstPageList,pszString,nRow,nColumn,FALSE);
					break;
				case 2: /* LastPage */
					FillData(&LastPageList,pszString,nRow,nColumn,FALSE);
					break;
				case 3: /* EveryPage */
					FillData(&EveryPageList,pszString,nRow,nColumn,bIncrement);
					break;
				case 4: /* NewPage */
					FillData(&NewPageList,pszString,nRow,nColumn,bIncrement);
					break;
				case 5: /* OldPage */
					FillData(&OldPageList,pszString,nRow,nColumn,bIncrement);
					break;
				default:
					/*
					* Tipo di dato non esistente
					*/
					nRetVal=2;
					break;
			}
		}
	}
	return (nRetVal);
}



/*
* Descrizione generale:
* E' la prima funzione che deve essere chiamata. Svolge i seguenti compiti:
*  # Apre il file su cui verranno memorizzati temporaneamente i dati da stampare nel formato 
*    di stampa 
*  # Alloca lo spazio necessario a contenere una pagina in memoria (vettore di stringhe)
*  # Stampa i dati fissi relativi alla prima pagina
* 
* Descrizione parametri:
* -pszFileName: nome del file sul quale memorizzare il'anteprima di stampa
* -nRows: numero delle righe di una pagina (offset 1)
* -nColumns: numero delle colonne di una pagina (offset 1)
* -nStartRow: riga dalla quale iniziare a stampare (offset 1)
* -nStopRow: ultima riga di stampa (offset 1)
* 
* Valori di ritorno:
*  il valore di ritorno e' un intero col seguente significato:
*		# 0 nessun errore rilevato
*		# 1 impossibile aprire il file in scrittura
*		# 2 impossibile allocare la memoria
*   # 3 errore nei parametri numerici
*/

int InitPrint(char *pszFileName,int nRows,int nColumns,int nStartRow,int nStopRow)
{
	int nRetVal=0;

	nPag=1;
	if (nRows<1 || nColumns<1 || nStartRow<1 || nStopRow>nRows) nRetVal=3;
	if (!nRetVal){
		if ((FilePointer=fopen(pszFileName,"w"))==(FILE *)NULL){
			nRetVal=1;
		}
		if (!nRetVal){
			/*
			* Alloca un vettore di nRows stringhe di lunghezza pari a nColumns
			*/
			StringVector=AllocArray(nRows,nColumns);
			if (StringVector==NULL) nRetVal=2;
		}
		if (!nRetVal){
			/*
			* Stampa i dati fissi relativi alla prima pagina.
			* Inizializza il numero righe e colonne della pagina nonche' la linea corrente.
			*/
			nRowsPage=nRows;
			nColumnsPage=nColumns;
			nFirstRow=nStartRow;
			nLastRow=nStopRow;
			nCurrentRow=nFirstRow;
			ClearPage();
			InitializeDataStructure();
		}
	}
	if (!nRetVal) bStato=TRUE;
	return(nRetVal);
}



/*
* Descrizione generale:
* Scrive sulla pagina in memoria a partire dalla posizione specificata da nRow,nColumn
* il contenuto di szString.
* Nel caso la stringa sfori le dimensioni (in larghezza) della pagina, la stringa viene
* troncata (non e' gestito il prosieguo a capo).
*
* Descrizione parametri:
* -pszString: contenuto del campo da stampare
* -nRow: riga della pagina sulla quale si vuole stampare il campo (offset 1)
* -nColumn: colonna dalla quale si vuole iniziare a stampare (offset 1)
*
* Valori di ritorno:
* La funzione ritorna TRUE se le operazioni vengono terminate con successo. FALSE in
* caso contrario.
*/

ep_bool_t PrintField(char *pszString,int nRow,int nColumn)
{
	int nSize;
	char *pszApp;
	ep_bool_t bRetVal=FALSE;

	if (bStato){
		if (nRow>0 && nColumn>0){
			nSize=min(nColumnsPage-nColumn,strlen(pszString));
			pszApp=StringVector[nRow-1]+nColumn-1;
			strncpy(pszApp,pszString,nSize);
			bRetVal=TRUE;
		}
	}
	return(bRetVal);
}



/*
* Descrizione generale:
* Stampa pszRow nella nuova riga successiva a quella corrente	e aggiorna la riga corrente. 
* Nel caso si tratti di un salto pagina, occorre  effettuare le seguenti operazioni:
*  # stampare i dati di OldPage (se opzione abilitata)
*  # scaricare la pagina sul file
*  # pulire la pagina
*  # stampare i dati di NewPage (se opzione abilitata)
*  # stampare i dati di EveryPage (se opzione abilitata)
* Nel caso pszRow contenga un numero di caratteri superiore alla larghezza della pagina, pszRow
* viene troncata alla larghezza della pagina. Viceversa (se la dimensione di pszRow e' inferiore
* alla larghezza della pagina) vengono aggiunti degli spazi in fondo alla riga.
*
* Descrizione parametri:
* -pszRow: riga da stampare 
* -nSkip: righe da saltare rispetto all'ultima riga stampata
*
* Valori di ritorno:
* La funzione ritorna la riga su cui verra' stampata la linea successiva se le operazioni 
* vengono terminate con successo, -1 se le operazioni non vengono terminate con successo.
* Le operazioni possono non terminare con successo nel caso si tenti di stampare una riga
* senza che le operazioni di inizializzazione (InitPrint) siano state terminate con suc-
* cesso.
*/

int PrintRow(char *pszRow,int nSkip,ep_bool_t bA4)
{
	int nRetVal=-1;
	int nSize;

	if (bStato){
		nSize=min(nColumnsPage,strlen(pszRow));
		if (nCurrentRow+nSkip>nLastRow){
			/*
			* Caso di cambio pagina
			*/
			nCurrentRow=nFirstRow;
			PutChangedPageInFile(FALSE,bA4);
		}
		else{
			/*
			* Caso di stessa pagina
			*/
			nCurrentRow+=nSkip;
		}
		strncpy(StringVector[nCurrentRow-1],pszRow,nSize);
		nRetVal=nCurrentRow;
	}
	return(nRetVal);
}



/*
* Descrizione generale:
* Salto pagina. Esegue la PutChangedPageInFile() e aggiorna la riga corrente.
*
* Descrizione parametri:
* -bPrintOldPage: se TRUE vengono stampati i dati di OldPage, se FALSE no.
* -bResetPageNumber: se TRUE il numero di pagina viene reimpostato a 1, se FALSE con-
*  tinua il conteggio
*
* Valori di ritorno:
* La funzione ritorna TRUE se le operazioni vengono terminate con successo. FALSE in
* caso contrario.
*/

ep_bool_t SkipPage(ep_bool_t bPrintOldPage,ep_bool_t bResetPageNumber,ep_bool_t bA4)
{
	ep_bool_t bRetVal=FALSE;

	if (bStato){
		bRetVal=TRUE;
		if (bPrintOldPage)
			PrintList(&LastPageList);
		PutChangedPageInFile(bResetPageNumber,bA4);
		nCurrentRow=nFirstRow;
	}
	return (bRetVal);
}


/*
* Descrizione generale:
* Funzione che ritorna il numero di pagina.
*/

int PageNumber()
{
	return nPag;
}


/*
* Descrizione generale:
* Esegue le seguenti operazioni:
*  # stampa i dati di ultima pagina
*  # scarica l'ultima pagina nel file
*  # chiude il file
*  # disalloca la memoria allocata per la pagina
*  # disalloca la memoria allocata per le liste dei dati
*
* Valori di ritorno:
* La funzione ritorna il numero di pagine stampate oppure un valore negativo in caso di problemi.
* -1: non erano state portate a termine con successo le operazioni di apertura del file di ante-
*			prima di stampa e di allocazione della memoria per la pagina.
* -2: non si riesce a chiudere il file di anteprima di stampa.
*/

int EndPrint(ep_bool_t bA4)
{
	int nRetVal=-1;
	int nRetClose;

	if (bStato){
		nRetVal=0;
		if (nPag==1){
			PrintList(&FirstPageList);
			PrintList(&EveryPageList);
		}
		PrintList(&LastPageList);
		PutPageInFile(bA4,FALSE);
		
		nRetClose=fclose(FilePointer);	/* Restituisce EOF se rileva errori */
		if (nRetClose) nRetVal=-2;
		free(StringVector);
		FreeList(&FirstPageList);
		FreeList(&LastPageList);
		FreeList(&EveryPageList);
		FreeList(&NewPageList);
		FreeList(&OldPageList);
	}
	if (!nRetVal)
		nRetVal=nPag;
	return(nRetVal);
}



/*
* Descrizione generale:
* Abilita o disabilita la stampa dei dati di tipo NewPage.
*
* Descrizione parametri:
* -Enable: se TRUE la funzione e' abilitata, se FALSE no.
*
* Valori di ritorno:
* La funzione non ritorna valori.
*/
void EnableNewPage(ep_bool_t Enable)
{
	bEnableNewPage=FALSE;
	if (Enable)
		bEnableNewPage=TRUE;
}



/*
* Descrizione generale:
* Abilita o disabilita la stampa dei dati di tipo OldPage.
*
* Descrizione parametri:
* -Enable: se TRUE la funzione e' abilitata, se FALSE no.
*
* Valori di ritorno:
* La funzione non ritorna valori.
*/
void EnableOldPage(ep_bool_t Enable)
{
	bEnableOldPage=FALSE;
	if (Enable)
		bEnableOldPage=TRUE;
}



/*
* Descrizione generale:
* Abilita o disabilita la stampa dei dati di tipo EveryPage.
*
* Descrizione parametri:
* -Enable: se TRUE la funzione e' abilitata, se FALSE no.
*
* Valori di ritorno:
* La funzione non ritorna valori.
*/
void EnableEveryPage(ep_bool_t Enable)
{
	bEnableEveryPage=FALSE;
	if (Enable)
		bEnableEveryPage=TRUE;
}



/*
* Descrizione generale:
* Effettua un salto pagina e imposta il conteggio delle pagine a partire dalla prima.
* Esegue le seguenti operazioni:
* -stampa i dati di tipo LastPage sulla pagina corrente
* -scarica la pagina corrente sul file
* -pulisce la pagina corrente
* -azzera il contatore di pagine
* -scrive sulla pagina corrente i dati di tipo FirstPage
*
* Valori di ritorno:
* La funzione non ritorna valori.
*/

void ResetPageNumber(ep_bool_t bA4)
{
	PrintList(&LastPageList);
	PutPageInFile(bA4,TRUE);
	ClearPage();
	nPag=1;
	PrintList(&FirstPageList);
}



/*
* Descrizione generale:
* Svuota la lista dei dati da stampare in seguito all'evento specificato in pszDataType.
*
* Descrizione parametri:
* -pszDataType indica quale lista di dati deve essere svuotata. Deve essere un tipo definito
*  (FirstPage,LastPage,EveryPage,NewPage,OldPage).
*
* Valori di ritorno:
* Restituisce TRUE se pszDataType e' un tipo di dato esistente, FALSE altrimenti.
*/

ep_bool_t DeleteData(char *pszDataType)
{
	int nTipoDato=0;
	ep_bool_t bRetVal=TRUE;

	if (!strcmp(pszDataType,"FirstPage")) nTipoDato=1;
	if (!strcmp(pszDataType,"LastPage")) 	nTipoDato=2;
	if (!strcmp(pszDataType,"EveryPage")) nTipoDato=3;
	if (!strcmp(pszDataType,"NewPage")) 	nTipoDato=4;
	if (!strcmp(pszDataType,"OldPage")) 	nTipoDato=5;
	switch (nTipoDato){
		case 1: /* FirstPage */
			FreeList(&FirstPageList);
			break;
		case 2: /* LastPage */
			FreeList(&LastPageList);
			break;
		case 3: /* EveryPage */
			FreeList(&EveryPageList);
			break;
		case 4: /* NewPage */
			FreeList(&NewPageList);
			break;
		case 5: /* OldPage */
			FreeList(&OldPageList);
			break;
		default:
			/*
			* Tipo di dato non esistente
			*/
			bRetVal=FALSE;
			break;
	}
	return(bRetVal);
}



/*
******************************************************************************************
*
* Funzioni interne al modulo
*
******************************************************************************************
*/



void FillData(LINKED_LIST *pList,char *pszString,int nRow,int nColumn,ep_bool_t bIncrement)
{
	int nSize;
	PDATA pElem;

	/*
	* Calcolo della memoria da allocare
	*/
	nSize=min(nColumnsPage-nColumn,strlen(pszString));
	pElem=(PDATA)malloc(sizeof(DATA));				
	pElem->pszString=(char *)malloc(nSize+1);
	strncpy(pElem->pszString,pszString,nSize);
	pElem->pszString[nSize]='\0';
	pElem->nRow=nRow;
	pElem->nColumn=nColumn;
	pElem->bIncrement=bIncrement;
	link_add(pList,pElem);
}



/*
* Restituisce un puntatore a un vettore di stringhe con vettore di dimensione nItem e stringhe di
* dimensione nSize
*/
char **AllocArray(int nItem, int nSize)
{
    int     nIndex;
    char    *pszTmp;
    char    *pszAlloc;
    char    **pszArray;

    if( nItem<=0 || nSize<=0 )
        return(NULL);

    /*
     *  Alloco lo spazio globale:
     *  puntatore + il puntato
     */
    pszAlloc=malloc((unsigned long)nItem*((unsigned long)sizeof(char *)+nSize+1));
    if( pszAlloc==NULL )
        return(NULL);

    /*
     *  Mi sposto in avanti dell'array di puntatori
     */
    pszTmp=pszAlloc+nItem*sizeof(char *);

    pszArray=(char **)pszAlloc;

    /*
     *  Loop per tutti gli elementi
     */
    for( nIndex=0; nIndex<nItem; nIndex++ )
    {
        pszArray[nIndex]=pszTmp;
        pszTmp += (nSize+1);
    }

    return((char **)pszAlloc);
}   /*  Fine xncAllocArray */



/*
* Questa funzione riempie la pagina con degli spazi.
*/
void ClearPage()
{
	int nIndex;

	for(nIndex=0;nIndex<nRowsPage;nIndex++){
		memset(StringVector[nIndex],' ',nColumnsPage);
		memset(StringVector[nIndex]+nColumnsPage,'\0',1);
	}
}



/*
* Stampa i dati relativi a una lista
*/
void PrintList(LINKED_LIST *pList)
{
	int nNum;
	int nLen;
	char szPag[20];
	char szBuffer[255];
	PDATA pData;

	pData=link_first(pList);
	while (pData){
		/*
		* Cerco se c'e' la stringa speciale nella stringa del dato: nel caso la sostituisco con 
		* il numero pagina.
		* Scrivo i dati sulla pagina
		*/
		sprintf(szPag,"%3d",nPag);
		strcpy(szBuffer,pData->pszString);
		SearchString(szBuffer,PAGINA,szPag);
		PrintField(szBuffer,pData->nRow,pData->nColumn);
		/*
		* Aggiorno il campo incrementale
		*/
		if (pData->bIncrement){
			nLen=strlen(pData->pszString);
			nNum=atoi(pData->pszString);
			sprintf(pData->pszString,"%0*d",nLen,nNum+1);
		}
		pData=link_next(pList,pData);
	}
}



/*
* Cerca la presenza della sottostringa pszString2 in pszString1 e, se la trova, la sostituisce
* con la stringa pszString3. Ritorna TRUE se la stringa e' stata trovata e sostituita, FALSE
* altrimenti. Sostituisce solo la prima sottostringa trovata.
*/
ep_bool_t SearchString(char *pszString1,char *pszString2,char *pszString3)
{
	int nLen1,nLen2;
	int nIndex=1;
	char *pszChar;
	char szPart1[255];
	char szPart2[255];
	ep_bool_t bFound=FALSE;
	
	nLen1=strlen(pszString1);
	nLen2=strlen(pszString2);
	pszChar=pszString1;
	while(!bFound && nIndex<=nLen1){
		if (!strncmp(pszChar,pszString2,nLen2)){
			bFound=TRUE;
			pszChar+=nLen2;
			strncpy(szPart1,pszString1,nIndex-1);
			szPart1[nIndex-1]='\0';
			strcpy(szPart2,pszChar);
			sprintf(pszString1,"%s%s%s",szPart1,pszString3,szPart2);
		}
		pszChar++;
		nIndex++;
	}
	return(bFound);
}



/*
* Scarica la pagina corrente in memoria eseguemdo le seguenti operazioni:
*  # se si tratta della prima pagina stampa i dati di FirstPage
*  # stampa i dati di OldPage sulla pagina corrente (se opzione abilitata)
*  # scarica la pagina corrente sul file
*  # pulisce la pagina corrente
*  # gestisce il conteggio del numero di pagina
*  # stampa i dati di NewPage sulla pagina corrente (se opzione abilitata)
*  # stampa i dati di EveryPage sulla pagina corrente (se opzione abilitata)
*/
void PutChangedPageInFile(ep_bool_t bResetPageNumber,ep_bool_t bA4)
{
	if (nPag==1){
		PrintList(&FirstPageList);
		PrintList(&EveryPageList);
	}
	if (bEnableOldPage)
		PrintList(&OldPageList);
	PutPageInFile(bA4,TRUE);
	ClearPage();
	nPag++;
	if (bResetPageNumber)
		nPag=1;
	if (bEnableNewPage)
		PrintList(&NewPageList);
	if (bEnableEveryPage)
		PrintList(&EveryPageList);
}




/*
* Copia la pagina corrente sul file di anteprima di stampa.
*/
void PutPageInFile(ep_bool_t bA4,ep_bool_t bFormFeed)
{
	int nIndex;


	if (bA4) {
		for(nIndex=0;nIndex<nRowsPage-1;nIndex++){
			fprintf(FilePointer,"%s\n",StringVector[nIndex]);
		}
		/* salto pagina */
		if(bFormFeed){
			fprintf(FilePointer,"%s",StringVector[nRowsPage-1]);
		} else {
			fprintf(FilePointer,"%s",StringVector[nRowsPage-1]);
		}
	} else {
		for(nIndex=0;nIndex<nRowsPage;nIndex++){
			fprintf(FilePointer,"%s\n",StringVector[nIndex]);
		}
	}
	ClearPage();
}



/*
* Disalloca l'area di memoria allocata per la lista dei dati.
*/
void FreeList(LINKED_LIST *pList)
{
	PDATA pData;

	while ((pData=link_pop(pList))!=NULL)
		free(pData);
}


void InitializeDataStructure()
{
	FirstPageList.last=NULL;
	FirstPageList.selected=NULL;
	FirstPageList.n_link=0;

	LastPageList.last=NULL;
	LastPageList.selected=NULL;
	LastPageList.n_link=0;

	EveryPageList.last=NULL;
	EveryPageList.selected=NULL;
	EveryPageList.n_link=0;

	NewPageList.last=NULL;
	NewPageList.selected=NULL;
	NewPageList.n_link=0;

	OldPageList.last=NULL;
	OldPageList.selected=NULL;
	OldPageList.n_link=0;
}


ep_bool_t VerifyAll(char *pszFileName)
{
	int nRetClose;
	ep_bool_t bRetVal=TRUE;
	FILE *fpCheck;

	if ((fpCheck=fopen(pszFileName,"w"))==(FILE *)NULL) {
 		bRetVal=FALSE;
	}
	fprintf(fpCheck,"\nFirstPageList\n");
	fprintf(fpCheck,"--------------\n\n");
	DataVerify(fpCheck,&FirstPageList);

	fprintf(fpCheck,"\nLastPageList\n");
	fprintf(fpCheck,"--------------\n\n");
	DataVerify(fpCheck,&LastPageList);

	fprintf(fpCheck,"\nEveryPageList\n");
	fprintf(fpCheck,"--------------\n\n");
  DataVerify(fpCheck,&EveryPageList);

	fprintf(fpCheck,"\nNewPageList\n");
	fprintf(fpCheck,"--------------\n\n");
  DataVerify(fpCheck,&NewPageList);

	fprintf(fpCheck,"\nOldPageList\n");
	fprintf(fpCheck,"--------------\n\n");
  DataVerify(fpCheck,&OldPageList);

	nRetClose=fclose(fpCheck);	/* Restituisce EOF se rileva errori */
	if (nRetClose) bRetVal=FALSE;
	return(bRetVal);
}


void DataVerify(FILE *fpCheck,LINKED_LIST *pList)
{
	PDATA pData;

	pData=link_first(pList);
	while (pData){
		fprintf(fpCheck,"Puntatore: %p\n",pData);
		fprintf(fpCheck,"Precedente: %p\n",pData->link.p);
		fprintf(fpCheck,"Successivo: %p\n",pData->link.n);
		fprintf(fpCheck,"String: %s\n",pData->pszString);
		fprintf(fpCheck,"Row Number: %d\n",pData->nRow);
		fprintf(fpCheck,"Column Number: %d\n",pData->nColumn);
		fprintf(fpCheck,"Increment: %d\n",pData->bIncrement);
		fprintf(fpCheck,"\n");
		pData=link_next(pList,pData);
	}
}
