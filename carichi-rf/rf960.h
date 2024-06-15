/*
* Modulo : rf.h
* -----------------
* Include file principale (costanti , enum , macros ...)
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19/06/2000
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1996
*/
#define DIR_SEP_CHAR					'/'

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define BOOL_TYPE_DEFINED
#endif

#ifndef UINT_TYPE_DEFINED
typedef unsigned int UINT;
#define UINT_TYPE_DEFINED
#endif

#define START_BOLD_VT100				"\033[1m"
#define END_BOLD_VT100					"\033[0m"

#define START_BOLD							_outtext(START_BOLD_VT100)
#define END_BOLD								_outtext(END_BOLD_VT100)

typedef unsigned char BYTE;

#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))

#define SCREEN_WIDTH 16
#define SCREEN_HEIGHT 4

#define MAX_OPERATORI 1024
#define MAX_TERM_RF 256
#define MAX_USERS	1024
#define MAX_PRODOTTI_IN_LISTA 256
#define MAX_PRODOTTI_A_VIDEO 4

#define SOCK_SEPARATOR  "|"	/* separatore */

enum FASI {
	GBLOGIN,
	GBCOLLO,
	GBPRELIEVO,
	GBCARICO_CESTA,
	GBFINE_PRELIEVO,
	GBTITOLO,
	GBCOLLO_PRELEVATO,
	GBCOLLO_INIZIATO,

	NUMERO_FASI,
};

enum STAZIONI_RF {
	STAZIONE_0,
	STAZIONE_1,
	STAZIONE_2,
	STAZIONE_3,
	STAZIONE_4,
	STAZIONE_5,
	STAZIONE_6,
	STAZIONE_7,

	NUM_STAZIONI_RF,
};

enum {
	STAZIONE_A,
	STAZIONE_B,
	STAZIONE_C,
	STAZIONE_D,
	STAZIONE_U,
	TUTTE_LE_STAZIONI,

	NUM_STAZIONI_GIOSTRA,
};
