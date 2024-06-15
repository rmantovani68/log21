/*
* @(#) grsy0tst.h 1.6 Thu Feb 22 18:52:13 MET 1996
*
* 16/05/95 GG : 1.1 . Aggiunte definizioni CNI.
* 30/11/95 GG : 1.2 . Altre definizioni CNI.
* 30/01/96 GG : 1.3 . Aggiunta l'operazione R_DYNLD_PROP.
* 31/01/96 GG : 1.4 . Aggiunti comandi di controllo statistica.
* 13/02/96 GG : 1.5 . Aggiunti comandi di copia multipla.
* 22/02/96 GG : 1.6 . Aggiunto un comando per azzerare la statistica.
*/

#define ER_NUM (short)0x601 
#define ER_TRT (short)0x602 
#define ER_PAR (short)0x603 
#define CR_DNR (short)0x300  
#define CR_RDY (short)0x400  

#define T_MODBUS 20  
#define T_SESSION 65  
#define T_QUESTION 67 
#define T_GET_DATA 68 
#define T_DEB_CHARGE 69 
#define T_CHARGE 70 
#define T_DEB_LECTURE 71 
#define T_LECTURE 72 

#define R_CONNECT 00 
#define R_CONTROL 10 
#define R_SURVEY 20 

#define CONTROL_APPLI 0 
#define CONTROL_STOP 1 
#define CONTROL_START 2 
#define CONTROL_SHELL 10 

#define R_BKS 23 
#define R_WILR 24 
#define R_WBOO 25 
#define R_WANA 26 
#define R_WTMR 27 
#define R_WMSG 28  
#define R_TMR 29 
#define R_TSTART 30 
#define R_TSTOP 31 
#define R_CC 32 
#define R_RT 33 
#define R_EC 34 
#define R_TUC 35 
#define R_TCC 36 
#define R_BKAC 37  
#define R_BKDE 38 
#define R_BKT 39 
#define R_BKDEL 40 
#define R_BK 41  
#define R_BKA 42 
#define R_TCW 43  
#define R_TCR 44 
#define R_GSTART 45 
#define R_GFREEZE 46 
#define R_GKILL 47  
#define R_GRST 48 
#define R_GSTATUS 49 
#define R_SLR 50 
#define R_GER 53 
#define R_GERACK 54 
#define R_UNLOCK 55 
#define R_LOCK 57 
#define R_UPMDF 62 
#define R_UPSYS 63 
#define R_CRC 69  
#define R_LOCKR 70 
#define R_STOP 71  
#define R_APPL 72  
#define R_RANY 80 
#define R_USREAD 81  
#define R_USADD 82 
#define R_WLIST 83 
#define R_USWANY 84 
#define R_LINK 90 


#ifdef DEF_COMPACT
#define MAX_TRAME 128
#define MAX_REQ 256
#else
#define MAX_TRAME 255
#define MAX_REQ 1024
#endif

/* CODICI RISERVATI CNI. */

#define R_TRACE_START 160	/* Lancia il trace. */
#define R_TRACE_STOP 161	/* Interrompe il trace. */
#define R_TRACE_STATUS 162	/* Riporta lo stato del trace. */
#define R_TRACE_LOAD 163	/* Copia una parte del buffer di trace. */
#define R_TRACE_ADD 164		/* Aggiunge una variabile alla lista. */
#define R_TRACE_CLEAR 165	/* Vuota la lista di trace. */
#define R_TRACE_SIGNAL 166	/* Segnala il completamento di una copia. */
#define R_TRACE_SETBUF 167	/* Imposta la dimensione dei suttobuffer. */

#define R_DYNLD_LOAD 170	/* Carica dinamicamente un modulo oggetto. */
#define R_DYNLD_UNLOAD 171	/* Scarica dinamicamente un modulo oggetto. */
#define R_DYNLD_STATUS 172	/* Stato del caricamento dinamico. */

#define R_KERMEM_COPY 173	/* Comunica il contenuto di un blocco di memoria
                         	 interna del kernel PLC. */

#define R_GET_TCYSTAT 174	/* Comunica il risultato della statistica del
                             tempo di ciclo (12 byte). */
#define R_DYNLD_PROP 175	/* Caratteristiche di un modulo. */

#define R_IF_ENSTAT 176		/* Stato della statistica. */
#define R_START_STAT 177	/* Abilitazione-disabilitazione statistica. */
#define R_GET_EXENAME 178	/* Fornisce il path completo dell'eseguibile. */
#define R_KERMEM_MCHK 179	/* Imposta convalida indirizzi per multiletture. */
#define R_KERMEM_MCOPY 180	/* Esegue una lettura multipla. */
#define R_CLEAR_STAT 181	/* Riazzeramento della statistica. */

