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
BOOL GetDatiOrdine(char *szOrdProg,PDATIORDINE pDatiOrdine,BOOL bStorico);
BOOL GetDatiCollo (char *szOrdProg,int nCollo,PDATICOLLO pDatiCollo,BOOL bStorico);
BOOL StampaPackingListBolla(char *szOrdProg, char *szPrinterName);
BOOL StampaPackingListCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,BOOL bStorico);
BOOL WriteTestataOrdine(FILE *fp,char *szOrdprog,BOOL bStorico);
BOOL WriteTestataCollo(FILE *fp,char *szOrdprog, int nCollo,BOOL bStorico);
BOOL StampaConteggioImballi(char *szDataDal, char *szDataAl, char *szCedola, char *szPrinterName);

/* printlabel.c */
int main(int argc,char** argv);

/* printrac.c */
int ReadPrintFieldsInfo(char *szCfgFile);
BOOL StampaEtichetteOrdine(FILE *fp,char *pszOrdProg,char *pszPrinterName,int nGID);
BOOL StampaEtichettaCollo(char *pszOrdProg,int nCollo,char *pszPrinterName,int nGID);
BOOL StampaEtichette(char *szSQLCmd,FILE *fp,char *szLabelPrinterName);
char *StampaListaCampo(char *szNomeCampo,char *szOrdProg,int nCollo);
char *StampaListaTitoli(char *szOrdProg,int nCollo);
char *StampaListaUbicazioni(char *szOrdProg,int nCollo);
BOOL StampaRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,int nGID);
BOOL StampaAllRAC(char *pszPrinterName,int nGID);
BOOL GetDSTSP(char *szOrdProg,char *szDSTSP);

/* printoldrac.c */
void PrintRC(int nRow, int nColumn, char *pszString);
void AllocaPagina(void);
void LiberaPagina(void);
void PulisciPagina(int nLen);
void ScaricaPagina(FILE *fp, int nLen);
void StampaEtichetta(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo);
void StampaFinePagina(PDATIORDINE pDatiOrdine, PDATICOLLO pDatiCollo,int nPagina, BOOL bStampaNumeroCollo);
void StampaFineCollo(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo);
void StampaFineOrdine(PDATIORDINE pDatiOrdine);
void InitFile(FILE *fp);
BOOL StampaBolla(FILE *fp, char *szOrdProg);
void StampaRiga(PDATIRIGA pDatiRiga, int nRiga);
BOOL PrintRACLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN);
BOOL PrintBollaRAC(char *pszPrinterName, char *szOrdProg);
BOOL PrintBolleRAC(char *pszPrinterName);
BOOL WriteTestataOrdine(FILE *fp,char *szOrdprog,BOOL bStorico);
BOOL WriteTestataCollo(FILE *fp,char *szOrdprog, int nCollo,BOOL bStorico);

/* printutil.c */
char *NotazioneConPunti(int nNum,char *pszString);
char *NotazioneConVirgola(int nNum,char *pszString);
char *NumberToLetter(int nNum,char *pszString);
void local_PrintFile(char *szFile,char *szPrinter,char *szTitle,BOOL bA2PS,int nWidth);
void PrintFileRaw(char *szPrintFile,char *szPrinterName);


/* printxab.c */
BOOL IsXAB(char *szStato);
char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
BOOL InserisciDatiEveryPageXAB(PDATIORDINE pDatiOrdine);
void InserisciDatiOldPageXAB(void);
void InserisciDatiNewPageXAB(PDATIORDINE pDatiOrdine);
BOOL InserisciDatiLastPageXAB(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
BOOL DatiVettore(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
BOOL StampaXAB(PDATIORDINE pDatiOrdine,BOOL bStorico);
BOOL StampaXABSpedizione(char *pszPrinterName,char *szOrdProg,BOOL bStorico);
BOOL StampaXABLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,BOOL bStorico);


/* printdoccons.c */
//BOOL IsXAB(char *szStato);
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
BOOL InserisciDatiEveryPageDC(PDATIORDINE pDatiOrdine,int nNumeroPezzi,int nValoreOrdine);
void InserisciDatiOldPageDC(void);
void InserisciDatiNewPageDC(PDATIORDINE pDatiOrdine);
BOOL InserisciDatiLastPageDC(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
BOOL DatiVettoreDC(PDATIORDINE pDatiOrdine,char *szCDVET,int nPos1,int nPos2,int nPos3);
BOOL StampaDC(PDATIORDINE pDatiOrdine,BOOL bStorico);
BOOL StampaDCSpedizione(char *pszPrinterName,char *szOrdProg,BOOL bStorico);
BOOL StampaDCLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,BOOL bStorico);


/* printxab_A4.c */
//BOOL IsXAB(char *szStato);
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
BOOL InserisciDatiEveryPageXAB_A4(PDATIORDINE pDatiOrdine);
void InserisciDatiOldPageXAB_A4(void);
void InserisciDatiNewPageXAB_A4(PDATIORDINE pDatiOrdine);
BOOL InserisciDatiLastPageXAB_A4(PDATIORDINE pDatiOrdine,int nQtaTot,int nPrzTot);
BOOL DatiVettore_A4(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
BOOL StampaXAB_A4(PDATIORDINE pDatiOrdine,BOOL bStorico);
BOOL StampaXABSpedizione_A4(char *pszPrinterName,char *szOrdProg,BOOL bStorico);
BOOL StampaPackingListLinea(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,BOOL bStorico);
BOOL CreateXAB_A4(char *pszPrinterName,char *szOrdProg,char *szFile,BOOL bStorico);

/* printpacklist.c */
//char *DateDDMMYYYY(char *szDateYYYYMMDD,char *szDateDDMMYYYY);
//char *DateDDMMYY(char *szDateYYYYMMDD,char *szDateDDMMYY);
BOOL InserisciDatiEveryPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo);
void InserisciDatiOldPagePL_PF(void);
void InserisciDatiNewPagePL_PF(PDATIORDINE pDatiOrdine);
BOOL InserisciDatiLastPagePL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,int nQtaTot,int nPrzTot);
BOOL DatiVettorePL_PF(PDATIORDINE pDatiOrdine,char *szCDVet,int nPos1,int nPos2,int nPos3);
BOOL StampaPL_PF(PDATIORDINE pDatiOrdine,PDATICOLLO pDatiCollo,BOOL bStorico);
BOOL StampaPackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,BOOL bStorico);
int CreatePackingListColloPrefincato(char *pszPrinterName,char *szOrdProg,int nCollo,char *szFile,BOOL bStorico);
BOOL StampaPackingListBollaPrefincato(char *pszPrinterName,char *szOrdProg);

/* printdist.c */
BOOL CambioScalo(PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo);
BOOL FillRow(char *pszRow,char *pszField,int nLen,int nColumn,int nAllineamento);
BOOL InitStampaDistinta(char *pszPrinterName,PDATIORDINE pDatiOrdine,PTOTALI pTotaliLinea,PTOTALI pTotaliScalo);
BOOL InserisciDatiEveryPageDistinta(PDATIORDINE pDatiOrdine);
void InserisciDatiLastPageDistinta(PTOTALI pTotali);
void InserisciDatiNewPageDistinta(void);
BOOL InserisciDatiOldPageDistinta(BOOL bSegue,char *pszTipoEvento,PDATIORDINE pDatiOrdine,PTOTALI pTotaliScalo);
BOOL StampaDistinta(char *pszPrinterName,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,BOOL bStorico);
BOOL StampaOrdineDistinta(PDATIORDINE pDatiOrdine,PTOTALI pTotaliLinea,PTOTALI pTotaliScalo,int *pnRigaDistinta);
/* printdatiordine.c */
//BOOL StampaDatiOrdine(char *szOrdProg, char *szPrinterName);
BOOL StampaDatiOrdine(char *szOrdProg, char *szPrinterName, BOOL bStampaSuFile);
BOOL StampaDatiCollo(char *szOrdProg, int nCollo, char *szPrinterName,FILE *fp_file,BOOL bStorico);
BOOL WriteTestataDatiOrdine(FILE *fp,char *szOrdprog,BOOL bStorico);
BOOL WriteTestataDatiCollo(FILE *fp,char *szOrdprog, int nCollo,BOOL bStorico);

