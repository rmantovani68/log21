/*
* stampa.h
*
* Funzioni di libreria esportate per la gestione delle stampe.
*
* Autore: Daniele Ravaioli
*
* Copyright A&L srl 2003-2021
*/

/*
* Exported functions
*/

/*
* Descrizione generale della libreria.
* Libreria di funzioni per agevolare la gestione di stampe e relative problematiche
* quali salti pagina, posizionamenti di scritte all'interno della pagina, numeri di
* pagina, scritte presenti su ogni pagina, solo sulla prima, solo sull'ultima, solo
* sul cambio pagina (vecchia e nuova pagina).
* La libreria produce come risultato finale la crazione di un file (detto di ante-
* prima di stampa), da inviare poi (a carico dell'utente) alla stampante.
* La InitPrint() e' la prima funzione che deve essere chiamata. Questa riceve in
* ingresso il nome del file di anteprima di stampa, le dimensioni della pagina e
* il numero di riga di partenza e di fine usati dalla funzione PrintRow(). La fun-
* zione PrintRow() scrive le stringhe che riceve in ingresso a partire dal numero
* di riga indicato in nStartRow di InitPrint() fino al numero di riga nStopRow.
* I salti riga sono gestiti automaticamente cosi' come i salti pagina. E' possibi-
* le inoltre stampare una stringa in una qualunque posizione della pagina, senza
* modificare il numero di riga corrente e gestire il salto pagina. 
* E' possibile poi usare la funzione InitData() per caricare dati da stampare in
* posizioni specificate al verificarsi di determinati eventi. 
* Gli eventi gestiti sono i seguenti:
*  -FirstPage dati da stampare solo sulla prima pagina
*  -LastPage dati da stampare solo sull'ultima pagina
*  -EveryPage dati da stampare su tutte le pagine
*  -NewPage dati da stampare quando si verifica un cambio pagina sulla pagina vecchia
*  -OldPage dati da stampare quando si verifica un cambio pagina sulla nuova pagina
* Le liste dati relative a NewPage,OldPage e EveryPage sono abilitabili e disabilita-
* bili a comando (tramite apposite funzioni). E' anche possibile svuotare la lista
* dei dati relativi a un evento tramite la funzione DeleteData().
* Se nella stringa dei dati e' presente l'identificatore @@ ad esso viene sostituito
* il numero di pagina.
* Al termine della stampa occorre chiamare la funzione EndPrint() per chiudere il fi-
* le di anteprima di stampa e disallocare la memoria di pagina corrente.
*/

int InitPrint(char *pszFileName,int nRows,int nColumns,int nStartRow,int nStopRow);
int InitData(char *pszDataType,char *pszString,int nRow,int nColulmn,ep_bool_t bIncremental);
int PrintRow(char *pszRow,int nSkip,ep_bool_t bA4);
ep_bool_t PrintField(char *pszField,int nRow,int nColumn);
ep_bool_t SkipPage(ep_bool_t bPrintOldPage,ep_bool_t bResetPageNumber,ep_bool_t bA4);
int EndPrint(ep_bool_t bA4);
void ResetPageNumber(ep_bool_t bA4);
ep_bool_t DeleteData(char *pszDataType);
void EnableNewPage(ep_bool_t Enable);
void EnableOldPage(ep_bool_t Enable);
void EnableEveryPage(ep_bool_t Enable);
ep_bool_t VerifyAll(char *pszFileName);
int PageNumber();

/*
* Internal functions
*/
void FillData(LINKED_LIST *List,char *pszString,int nRow,int nColumn,ep_bool_t bIncrement);
char **AllocArray(int nItem, int nSize);
void ClearPage(void);
void PrintList(LINKED_LIST *List);
ep_bool_t SearchString(char *pszString1,char *pszString2,char *pszString3);
void PutChangedPageInFile(ep_bool_t bResetPageNumber,ep_bool_t bA4);
void PutPageInFile(ep_bool_t bA4,ep_bool_t bFormFeed);
void FreeList(LINKED_LIST *List);
void InitializeDataStructure(void);
void DataVerify(FILE *fpCheck,LINKED_LIST *pList);
