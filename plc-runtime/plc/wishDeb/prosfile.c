#define STRICT
#define OEMRESOURCE
#include <windows.h>
#include <keylibcni.h>

#include <tcl.h>
#include <tk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *****************************************************************************
 *      ProsAvviaProtezioneCmd
 *
 *              Funzione di inizializzazione protezione e
 *              testa lo stato dell'applicazione da chiamare all'inizio
 *              all' inizio della applicazione
 *
 *****************************************************************************
 */
int ProsAvviaProtezioneCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
DWORD cbData, dwType ;
HKEY NomeRegistro ;
char Path[1024] ;	/* ritorno valore registro */
char Registro[1024] ;
char dirett[64];
char *chiaveReg ;
int  app,prosTime;

 dwType = REG_SZ ;   /* ritorna una stringa NUll TERM. */
 cbData= 1024 * sizeof(char) ;

 chiaveReg="Software\\Cni\\" ;   /* chiave generica */
 memset(dirett,0,sizeof(dirett));
 LeggeStringa ("PRE",&dirett[0],sizeof(dirett));
 if(!strcmp(dirett,"TLA"))
 	sprintf(Registro, "%s%s", chiaveReg, "TLA");
  else
 	sprintf(Registro, "%s%s", chiaveReg, "PDS");
 
 RegCreateKey(HKEY_CURRENT_USER, Registro, &(NomeRegistro));

 if ( NomeRegistro ) 
   {
   /* leggo valore registro richiesto  */
   if ( RegQueryValueEx(NomeRegistro, "PATH", NULL, &dwType, (LPVOID)Path,
			&cbData) != ERROR_SUCCESS ) 
     sprintf(interp->result,"-1");
   else
     {
     if(AvviaStringa((HINSTANCE)NULL) == PP_SUCCESS)
       {
        app=CaricaStringa(2,&prosTime);
        switch (app)    
          {
          case CHIERR:        //      libreria NON iniziali
     	    sprintf(interp->result,"CHIERR");
          break;
          case APP_DEM:       //      applicativo in demo mode (DEMO)
     	    sprintf(interp->result,"APP_DEM");
          break;
          case APP_FUL:       //      applicativo Attivo (FULL)
     	    sprintf(interp->result,"APP_FUL");
          break;
          case APP_EXP:       //      applicativo Expired (EXPIRED)
     	    sprintf(interp->result,"APP_EXP");
          break;
          };
       }
     }

   RegCloseKey(NomeRegistro);
 }

  return TCL_OK;
}

/*
 *****************************************************************************
 *      ProsVerificaProtezioneCmd
 *
 *              Funzione che verifica lo stato
 *              dell'applicazione
 *
 *****************************************************************************
 */
int ProsVerificaProtezioneCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
int  app;

  app=CaricaCarattere(2);
  switch (app)    
    {
    case CHIERR:        //      libreria NON inizializzata
      sprintf(interp->result,"CHIERR");
    break;
    case VBIT_HIG:       //      opzione attiva
      sprintf(interp->result,"VBIT_HIG");
    break;
    case VBIT_LOW:       //      opzione non attiva
      sprintf(interp->result,"VBIT_LOW");
    break;
    };

  return TCL_OK;
}

/*
 *****************************************************************************
 *      ProsJollyStringaCmd
 *
 *              Funzione di update del file di licenza
 *              da chiamare dopo la inuizializzazione
 *
 *****************************************************************************
 */
int ProsJollyStringaCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
  JollyStringa();
  return TCL_OK;
}

/*
 *****************************************************************************
 *      ProsChiudiProtezione
 *
 *              Funzione di chiusura ed aggiornamento file di licenza
 *              da chiamare obbligatoriamente all' uscita
 *
 *****************************************************************************
 */
int ProsChiudiProtezioneCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv) 
{
  ChiudiStringa();
  return TCL_OK;
}

