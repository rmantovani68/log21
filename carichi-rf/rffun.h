/*
* Modulo : rffun.h
* -------------------
* Include file per la dichiarazione di tutti i prototipi di funzione
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19-06-2000
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1996-2000
*/

/* 
* rf
*/
int main(int argc,char **argv);
void ProcessKey(int nKeyPressed);
void do_forzatura(void);

/* 
* rfbox
*/
void ClearScreen(void);
void sane_exit(int retval);
void SafeIntFunc();
void SigSafeIntFunc(int);
void DisplayMessage(int nRow,char *szMessage,ep_bool_t bReverse);
void FlashMessage(char *szMessage);
void ReverseVideo(ep_bool_t bReverse);
// Schermate di Login
ep_bool_t LoginUser(void);
int MainMenu(void);
ep_bool_t YesNoMenu(char *szQuestion);
ep_bool_t FormIngressoMerce(void);
ep_bool_t FormSpedizioneMerce(void);
ep_bool_t FormConteggioMerce();
int FormDistinta(void);
int SetDistintaMenu(void);
int FormSetUdcInDistinta(int nDistinta);
ep_bool_t WarningMenu(char *szMessage);

/* 
* rffun
*/
// Funzioni Generiche
ep_bool_t ReadConfiguration(char *szCfgFileName,ep_bool_t bReadProcInfo);
ep_bool_t SendProcessMessage(int nDest,int nSrce,int nMsgCode,char *szText);
//ep_bool_t CheckLogin(char *szUserId,char *szPassword);
//ep_bool_t CaricoProdotto(char *szCDPRO,int nQTPRO);
//ep_bool_t MovimentaProdotto(char *szCDPRO,int nQTPRO,char *szCausale,int nDistinta,int nCedola,char *szUDC,ep_bool_t bGiacenza);
//int InsertDistinta(void);
//ep_bool_t CheckDistinta(int nDist);
//ep_bool_t CaricaUDC(char *szUDC,char *szCDPRO,int nQTPRO);
//ep_bool_t InsertProdottoCatalogo(char *szCDPRO);

