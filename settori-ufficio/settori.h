/*
* settori.h
* 
* Progetto Easy Picking 2.0 : Europharco
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define FALSE	0
#define TRUE 	1
#define BOOL_TYPE_DEFINED
#endif

#ifndef MIN_MAX_DEFINED
#define max(a,b)	((a)>=(b)?(a):(b))
#define min(a,b)	((a)<=(b)?(a):(b))
#endif

#ifndef BYTE_TYPE_DEFINED
typedef unsigned char BYTE;
#define BYTE_TYPE_DEFINED
#endif

enum StatoRiga {
	ATTESA_PRELIEVO,
	PRELEVATA,
};
enum ModiDisplay {
	NORMAL,
	BLINK,
};

enum RigaDisplay {
	TUTTO,
	RIGA_1,
	RIGA_2,
};

enum StatiSettore {
	ATTESA,
	PRELIEVO_IMBALLI,
	PRELIEVO_COPIE,
	PASSA_SETT_SUCC,
	CHIUSURA_COLLO,
	FINE_ORDINE,
	DISABILITATO,

	NUM_STATI_SETTORE,
};

enum TipiSettore {
	SETTORE_START,
	SETTORE_INTERMEDIO,
	SETTORE_END,
};

enum StatiOrdProd {
	EVASO,
	DA_EVADERE,
	IN_EVASIONE,
};

enum TipiOrdine {
	ORDINE_AUTOMATICO,
	ORDINE_MANUALE,

	NUM_TIPI_ORDINE,
};


#define MAX_UBICAZIONI    10000
#define MAX_CEDOLE        32

#define MAX_ISOLE       32
#define MAX_SETTORI    100
#define MAX_DISPLAYS   100

#define MAX_PKL	         4	/* numero max di moduli PKL presenti */
#define MAX_RIGHE_PKL		64
#define MAX_COLONNE_PKL	64
