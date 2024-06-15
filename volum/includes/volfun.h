/*
* volfun.h
* 
* Progetto Easy Picking 4.0
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

/*
* volfun.c
*/
int AnalisiVolumetrica(char cStato,ep_bool_t bControl,int nProcIndex,ep_bool_t bTest);
ep_bool_t AnalisiVolumetricaOrdine(char *szOrdProg,ep_bool_t bCheckTables,ep_bool_t bPallet,ep_bool_t bForzatura,ep_bool_t bProdottiNonUbicati,int nProcIndex,ep_bool_t bTest);
ep_bool_t CambiaFlagRigaRicevuta(char *pszOrdProg,char *pszCodiceProdotto,char cFlag);
int CmpImballo(PIMBALLO pImb1,PIMBALLO pImb2);
int CmpInt(int *pInt1,int *pInt2);
gint CmpRigaOrdine(gconstpointer P1,gconstpointer P2);
char *ColloBarcode(PCOLLO pCollo);
ep_bool_t CreazioneCollo(PCOLLO pCollo,int nCollo,ep_bool_t bPallet);
ep_bool_t InsertColProd(PCOLLO pCollo,char cStato);
ep_bool_t InsertRigProd(PEP_RIGA pRiga, char cStato);
int InsertRiga(PORDINE pOrdine,PCOLLO pCollo,PEP_RIGA pRiga,ep_bool_t bPallet);
void ReadConfiguration(ep_bool_t bReadProcInfo);
int ReadImballi(PIMBALLI pImb);
void SortImballi(PIMBALLI pImb);
ep_bool_t UpdateOrdine(PORDINE pOrdine);

/*
* volutil.c
*/
#ifdef VEDI_EP_COMMON
char *LeftStr(char *szString,int nLen);
char *SubStr(char *szString,int nPos,int nLen);
char *StrTrimLeft(char *str);
char *StrTrimRight(char *str);
char *StrTrimAll(char *str);
char *GetDate(long time,char *szDateString);
char *GetDateYYYYMMDD(long time,char *szDateString);
#endif
void SafeIntFunc();

/*
* volum.c
*/
int main(int argc,char** argv);

