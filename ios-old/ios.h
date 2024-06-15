/*
* ios.h : gestione IOS
* 
* Progetto Mondadori Picking
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

/*
* definizione della struttura ios_info_t
* --------------------------------------------------------------------------
*/
#ifndef HAS_IOS_CNI
#define MAX_IOS_CPU 4
struct ios_info_t {
	unsigned char base[4000];          /* Indirizzo dell'area condivisa. */
	int board_size;	                   /* Dimensione dell'area condivisa. */
	unsigned char cpu[MAX_IOS_CPU][4000];   /* Indirizzi dei controllori. */
	int cpu_size[MAX_IOS_CPU];         /* Dimensioni delle aree dei controllori */
	unsigned char *nvram;              /* Indirizzo della RAM non volatile. */
	int nvram_size;                    /* Dimensione della RAM non volatile. */
	int e2_size;                       /* Dimensione dell'E2 seriale. */
};
#endif
/*
* --------------------------------------------------------------------------
*/

#define LOBYTE(w)	((BYTE)(w))
#define HIBYTE(w)	((BYTE)(((UINT)(w)>>8) & 0xff))

#define MAX_IOS	     4	/* numero max di schede IOS presenti */
#define MAX_PKL	     4	/* numero max di moduli PKL presenti */
#define IOS_TIMEOUT 5000   /* attesa risposta da IOS */

typedef struct tagPKLStruct{
	int nIOS;
	int nCPU;
	int nModulo;
} PKLSTRUCT, *PPKLSTRUCT;

typedef struct tagCfgStruct{
	char szLanguage[80];
	int nDelay;
	int nDebugLevel;
	int nIOS;					/* numero di CPU IOS presenti */
	int nSimulazione; /* Simulazione della pressione tasto */
	int nDelaySimulazione; /* attesa pressione tasto simulata */
	struct ios_info_t IOSInfo[MAX_IOS];
	int nFdIOS[MAX_IOS];
	int nPKLNumber;
	PKLSTRUCT PKL[MAX_PKL];
	char szPathExe[128];
	char szPathTrace[128];
} CFGSTRUCT, *PCFGSTRUCT;

/*
* Set di macro per l'I/O IOS
*/
#ifdef HAS_IOS_CNI
#define IOS_TX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((Cfg.IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(n_byte))
#define IOS_RX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((Cfg.IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(0x300+(n_byte)))
#else
#define IOS_TX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((Cfg.IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(n_byte))
#define IOS_RX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((Cfg.IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(0x300+(n_byte)))
#endif

/*
* Inizio dichiarazione dei prototipi
*/
void SafeIntFunc();
char *GetTime(long time,char *szTimeString);
char *GetDate(long time,char *szDateString);
ep_bool_t SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);

#define PKL_NULL_CMD  0x00
#define PKL_LIGHT_ON  0x01
#define PKL_LIGHT_OFF 0x02
#define PKL_RESET	    0x03
#define PKL_TEST	    0x04
#define PKL_FREEZE	  0x05

#define PKL_BUTTON_PRESSED  0x40
#define PKL_LIGHT_DAMAGED   0x41

#define DPY_BCD_03             0x11
#define DPY_BCD_03_BLINK       0x12
#define DPY_BCD_47             0x13
#define DPY_BCD_47_BLINK       0x14
#define DPY_ASCII_DGT          0x15
#define DPY_ASCII_DGT_BLINK    0x16
#define DPY_PRESET_TIMER_BLINK 0x17
#define DPY_BCD_03_FLAG        0x18
#define DPY_BCD_47_FLAG        0x19

#define BCD_0	0
#define BCD_1	0
#define BCD_2	0
#define BCD_3	0
#define BCD_4	0
#define BCD_5	0
#define BCD_6	0
#define BCD_7	0
#define BCD_8	0
#define BCD_9	0
#define BCD_SPACE	10
#define BCD_MINUS	11
#define BCD_POINT	12
#define BCD_ASCI1	13
#define BCD_ASCI2	14
#define BCD_ASCI3	15
