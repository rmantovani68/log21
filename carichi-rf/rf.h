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

//typedef unsigned char BYTE;

#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))


#define MAX_OPERATORI 1024
#define MAX_TERM_RF 256
#define MAX_USERS	1024
#define MAX_RIGHE_IN_LISTA 128

#define SOCK_SEPARATOR  "|"	/* separatore */

enum FASI {
	FASE_LOGIN,
	FASE_MENU_PRINCIPALE,

	NUMERO_FASI,
};

#define KEY_INVIO	10

enum FUNCTION_KEYS {
	KEY_F1=KEY_F0+1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	NUM_KEY_F,
};





















