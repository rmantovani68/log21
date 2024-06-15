/*
* settfun.h
* 
* Progetto Easy Picking 2.0 : Europharco
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
/*
* settutil.c
*/
char *SubStr(char *szString,int nPos,int nLen);
char *RightStr(char *szString,int nLen);
char *LeftStr(char *szString,int nLen);
char *SubStr(char *szString,int nPos,int nLen);
char *StrTrimLeft(char *str);
char *StrTrimRight(char *str);
char *StrTrimAll(char *str);
char *i_strtrimall(char *str);
char *GetTime(char *szTimeString);
char *GetDate(char *szDateString);
char *GetDateYYYYMMDD(char *szDateString);
char *GetTimeHHMMSS(char *szTimeString);
void SaveStateIntFunc();
void SafeIntFunc();
void TerminateProcess(int nProcID);
ep_bool_t SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
/*
* settfun.c
*/
void ReadConfiguration(ep_bool_t bReadProcInfo);
void ReadProcInfo(char *szFileConfig);
ep_bool_t FileExists(char *szFileName);
ep_bool_t GetOrdine(PORDINE pOrdine,int nIsola,int nCedola);
ep_bool_t OrdineProducibile(PORDINE pOrdine);
ep_bool_t InizioPrelievoSettore(PORDINE pOrdine,PSETTORE pSettore);
ep_bool_t GestioneSettore(EVENTO *pEvento);
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
void ResetSettori(ep_bool_t bLinee);
void ResetSettore(int nIsola,int nSettore,ep_bool_t bLinee);
ep_bool_t CambiaFlagOrdine(char *szOrdProg, char cFlag);
ep_bool_t CambiaStatoOrdine( char *szOrdProg, char cStato);
ep_bool_t CambiaStatoCollo(char *szOrdProg,int nCollo,char cStato);
char StatoOrdine(char *szOrdProg);
char StatoCollo(char *szOrdProg,int nCollo);
ep_bool_t CambiaStatoColliOrdine( char *szOrdProg, char cStato);
ep_bool_t CambiaStatoRigheOrdine(char *szOrdProg,int nCollo,char cStato);
ep_bool_t UpdateRigaOrdine(PRIGA_PRELIEVO pRigaOrdine,PSETTORE pSettore);
void ScongelaPKL(void);
void CongelaPKL(void);
int GetSettore(int nIsola,int nSettore);
ep_bool_t RestoreState(char *pszFileName);
ep_bool_t SaveState(char *pszFileName);
char *GetDisplayData(char *pszOrdProg);
ep_bool_t ProduzioneOrdine(PORDINE pOrdine);
void SetStatoLinea(int nStatoLinea);
int GetStatoLinea(void);
int ReadDisplaysInfo(PDISPLAYSTRUCT pDisplays);
ep_bool_t ReadSettoriInfo(int nNumeroSettori);
int GetDisplay(int nDisplay,int nIsola);


/*
* settori.c
*/
int main(int argc,char** argv);

