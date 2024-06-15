/*
* printfun.h
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
*
* Copyright A&L srl 2003-2021
*/


/* printdist.c */

/* printfun.c */
void SafeIntFunc(int s);
void ReadConfiguration(void);
void ReadProcInfo(char *szFileConfig);
ep_bool_t GetDatiOrdine(char *szOrdProg,PDATIORDINE pDatiOrdine,ep_bool_t bStorico);
ep_bool_t GetDatiCollo (char *szOrdProg,int nCollo,PDATICOLLO pDatiCollo,ep_bool_t bStorico);
ep_bool_t StampaPackingListBolla(char *szOrdProg, char *szPrinterName);
ep_bool_t StampaPackingListCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,ep_bool_t bStorico);
ep_bool_t WriteTestataOrdine(FILE *fp,char *szOrdprog,ep_bool_t bStorico);
ep_bool_t WriteTestataCollo(FILE *fp,char *szOrdprog, int nCollo,ep_bool_t bStorico);
char *TipoVettoreBarcode(DBresult *DBRes,int nIndex);
char *VettoreBarcode(DBresult *DBRes,int nIndex);
char *BarcodeSegnacollo_OLD(DBresult *DBRes,int nIndex);
char *BarcodeSegnacollo(DBresult *DBRes,int nIndex);

/* printlabel.c */
int main(int argc,char** argv);

/* printrac.c */
int ReadPrintFieldsInfo(char *szCfgFile);
ep_bool_t StampaEtichetteOrdineStorico(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID);
ep_bool_t StampaEtichettaColloStorico(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID);
ep_bool_t StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID);
ep_bool_t StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID);
ep_bool_t StampaEtichetteStorico(char *szSQLCmd,FILE *fp,char *szLabelPrinterName);
ep_bool_t StampaEtichette(char *szSQLCmd,FILE *fp,char *szLabelPrinterName);
char *StampaListaCampo(char *szNomeCampo,char *szOrdProg,int nCollo);
char *StampaListaTitoli(char *szOrdProg,int nCollo);
char *StampaListaUbicazioni(char *szOrdProg,int nCollo);
ep_bool_t StampaRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char * szCDVET,int nGID);
ep_bool_t StampaAllRAC(char *pszPrinterName,int nGID);
ep_bool_t GetDSTSP(char *szOrdProg,char *szDSTSP,ep_bool_t bStorico);
ep_bool_t GetTestoDDT(char *szOrdProg,int nCollo,char *szTestoDDT);

/* printutil.c */
char *NotazioneConPunti(int nNum,char *pszString);
char *NotazioneConVirgola(int nNum,char *pszString);
char *NumberToLetter(int nNum,char *pszString);
void LocalPrintFile(char *szFile,char *szPrinter,char *szTitle,ep_bool_t bA2PS,int nWidth);
void PrintFileRaw(char *szPrintFile,char *szPrinterName);


/* printxab.c */
ep_bool_t IsXAB(char *szStato);
char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
ep_bool_t InserisciDatiEveryPageXAB(PDATIORDINE pDatiOrdine);
void InserisciDatiOldPageXAB(void);
void InserisciDatiNewPageXAB(PDATIORDINE pDatiOrdine);
ep_bool_t InserisciDatiLastPageXAB(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
ep_bool_t DatiVettore(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
ep_bool_t StampaXAB(PDATIORDINE pDatiOrdine,ep_bool_t bStorico);
ep_bool_t StampaXABSpedizione(char *pszPrinterName,char *szOrdProg,ep_bool_t bStorico);
ep_bool_t StampaXABLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,ep_bool_t bStorico);


/* printdoccons.c */
//ep_bool_t IsXAB(char *szStato);
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
ep_bool_t InserisciDatiEveryPageDC(PDATIORDINE pDatiOrdine,int nNumeroPezzi,int nValoreOrdine);
void InserisciDatiOldPageDC(void);
void InserisciDatiNewPageDC(PDATIORDINE pDatiOrdine);
ep_bool_t InserisciDatiLastPageDC(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
ep_bool_t DatiVettoreDC(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3);
ep_bool_t StampaDC(PDATIORDINE pDatiOrdine,ep_bool_t bStorico);
ep_bool_t StampaDCSpedizione(char *pszPrinterName,char *szOrdProg,ep_bool_t bStorico);
ep_bool_t StampaDCLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico);


/* printxab_A4.c */
//ep_bool_t IsXAB(char *szStato);
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
ep_bool_t InserisciDatiEveryPageXAB_A4(PDATIORDINE pDatiOrdine);
void InserisciDatiOldPageXAB_A4(void);
void InserisciDatiNewPageXAB_A4(PDATIORDINE pDatiOrdine);
ep_bool_t InserisciDatiLastPageXAB_A4(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
ep_bool_t DatiVettore_A4(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
ep_bool_t StampaXAB_A4(PDATIORDINE pDatiOrdine,ep_bool_t bStorico);
ep_bool_t StampaXABSpedizione_A4(char *pszPrinterName,char *szOrdProg,ep_bool_t bStorico);
ep_bool_t StampaPackingListLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico);
ep_bool_t CreateXAB_A4(char *pszPrinterName,char *szOrdProg,char *szFile,ep_bool_t bStorico);

/* printpacklist.c */
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
ep_bool_t InserisciDatiEveryPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo);
void InserisciDatiOldPagePL_PF(void);
void InserisciDatiNewPagePL_PF(PDATIORDINE pDatiOrdine);
ep_bool_t InserisciDatiLastPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,int nQtaTot,int nPrzTot);
ep_bool_t DatiVettorePL_PF(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
ep_bool_t StampaPL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,ep_bool_t bStorico);
ep_bool_t StampaPackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,ep_bool_t bStorico);
int CreatePackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,char *szFile,ep_bool_t bStorico);
ep_bool_t StampaPackingListBollaPrefincato(char *pszPrinterName,char *szOrdProg);

/* printdist.c */
ep_bool_t CambioScalo(PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo);
ep_bool_t FillRow(char *pszRow,char *pszField,int nLen,int nColumn,int nAllineamento);
ep_bool_t InitStampaDistinta(char *pszPrinterName,PDATIORDINE pDatiOrdine,PTOTALI pTotaliLinea,PTOTALI pTotaliScalo);
ep_bool_t InserisciDatiEveryPageDistinta(PDATIORDINE pDatiOrdine);
void InserisciDatiLastPageDistinta(PTOTALI pTotali);
void InserisciDatiNewPageDistinta(void);
ep_bool_t InserisciDatiOldPageDistinta(ep_bool_t bSegue,char *pszTipoEvento,PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo);
ep_bool_t StampaDistinta(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,char *szANNO,ep_bool_t bStorico);
ep_bool_t StampaOrdineDistinta(PDATIORDINE pDatiOrdine,PTOTALI pTotaliLinea,PTOTALI pTotaliScalo,int *pnRigaDistinta);

