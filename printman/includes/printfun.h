/*
* printfun.h
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani / Luca Guerrini
*
* Copyright A&L srl 2003-2021
*/


/*
* printlabel.c
*/
int main(int argc,char** argv);

/*
* printfun.c
*/
ep_bool_t ReadConfiguration(char *szCfgFileName);
ep_bool_t StampaSpedizione(char *szKey,char *szPrinter);
ep_bool_t StampaDistintaLinea(char *szKey,char *szPrinter);
ep_bool_t StampaFrontespizioBancale(char *szKey,char *szPrinter);
void SafeIntFunc();
