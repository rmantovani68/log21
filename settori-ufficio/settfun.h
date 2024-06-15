/*
* settfun.h
* 
* Progetto Easy Picking 2.0 
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

/*
* settfun.c
*/
void ReadConfiguration(BOOL bReadProcInfo);
void ReadProcInfo(char *szFileConfig);
BOOL FileExists(char *szFileName);
BOOL GetOrdine(PORDINE pOrdine,int nCedola);
BOOL OrdineProducibile(PORDINE pOrdine);
BOOL InizioPrelievoSettore(PORDINE pOrdine,PSETTORE pSettore);
BOOL GestioneSettore(EVENTO *pEvento);
void ReadOperatori(void);
int ReadUbicazioni(PUBICAZIONI pUbi);
void SortUbicazioni(PUBICAZIONI pUbi);
PUBICAZIONE SearchUbicazione(PUBICAZIONI pUbi,int nIOS,int nCPU,int nModulo,int nRowIndex,int nColIndex);
int CmpUbicazione(PUBICAZIONE pUbi1,PUBICAZIONE pUbi2);
void UpdateDisplay(PDISPLAYSTRUCT pDisplay,int nRiga);
void DisplayWriteString(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString);
int GetBCDNibble(unsigned char cChar);
void DisplayWriteStringBCD(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString,unsigned char nDot,unsigned char nBlink);
void SetLampada(int nIOS,int nCPU,int nModulo,int nRiga,int nColonna);
void ResetLampada(int nIOS,int nCPU,int nModule,int nRowIndex, int nColumn);
void ResetLuci(void);
void ResetSettori(void);
void ResetSettore(int nIsola,int nSettore);
BOOL CambiaFlagOrdine(char *szOrdProg, char cFlag);
BOOL CambiaStatoOrdine( char *szOrdProg, char cStato);
BOOL CambiaStatoCollo(char *szOrdProg,int nCollo,char cStato);
BOOL CambiaStatoColliOrdine( char *szOrdProg, char cStato);
BOOL CambiaStatoRigheOrdine(char *szOrdProg,int nCollo,char cStato);
BOOL UpdateRigaOrdine(PRIGA_PRELIEVO pRigaOrdine,PSETTORE pSettore);
void ScongelaPKL(void);
void CongelaPKL(void);
int GetTipoOrdine(char *pszOrdine);
int GetSettore(int nIsola,int nSettore);
BOOL RestoreState(char *pszFileName);
BOOL SaveState(char *pszFileName);
char *GetDisplayData(char *pszOrdProg);
BOOL ProduzioneOrdine(PORDINE pOrdine);
void SetStatoLinea(int nStatoLinea);
int GetStatoLinea(void);
int ReadDisplaysInfo(PDISPLAYSTRUCT pDisplays);
void ReadSettoriInfo(int nNumeroSettori);
int GetDisplay(int nDisplay,int nIsola);
void SaveStateIntFunc();
void SafeIntFunc();


/*
* settori.c
*/
int main(int argc,char** argv);

