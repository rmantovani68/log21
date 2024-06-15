/*
* modulo : dbfun.h
* -----------------
* Include file per libreria di gestione database
*
* Progetto Picking - DB Management - PostgreSQL
*
* Data creazione 13/12/1995
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 07-07-1998 : Aggiunta compilazione condizionale per CodeBase 4.5/5.1
* rm 18-01-1999 : Aggiunta la gestione multi linea per DeAgostini
* rm 23-02-1999 : Aggiunta struttura DB Array
* rm 16-06-2000 : Eliminati i riferimenti a CodeBase - si usa PGSQL
* -------------   -------------------------------------------------------
*/
#ifndef _dbfun_h_
#define _dbfun_h_

/* bitmask per campi */
#define DISPLAY_MASK  0x01
#define EDIT_MASK     0x02
#define IMPORT_MASK	  0x04
#define EXPORT_MASK	  0x08
#define SELECT_MASK	  0x10
#define KEY_MASK	    0x20

/* bitmask per tabelle */
#define INSERT_MASK	    0x01
#define DELETE_MASK	    0x02
#define CHANGE_MASK	    0x04

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE 	1
#endif
#define BOOL_TYPE_DEFINED
#endif

#define MAX_FIELDS_NUMBER	256
#define MAX_TAGS_NUMBER		64
#define FIELD_TITLE_LEN   40
#define FIELD_DESCR_LEN   80

ep_bool_t ConnectDataBase(char *szHost,char *szPort,char *szDataBase);
ep_bool_t ConnectDataBaseUser(char *szHost, char *szPort, char *szDataBase, char *szUserName, char *szPassword);
ep_bool_t TestDataBaseConnection(ep_bool_t bReset);
void DisconnectDataBase(void);
PGresult *PGExecSQL (ep_bool_t bTrace,char *format, ...) __attribute__ ((format(printf,2,3)));


#endif /* _dbfun_h_ */
