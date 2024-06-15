/*
* plcsock.h
* include file per plcsock.c
*/

#define PLC_SERVICE_PORT 8039

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

#define MAX_RCV_SIZE	1024

/*
* Maschere per l'interpretazione del campo "flags" delle caratteristiche
* di un segnale di scambio.
*/

#define SHV_MODE_CONTIG 0x04 /* Alloca una matrice contigua. */
#define SHV_MODE_EDGE 0x08 /* Il Plc deve essere sensibile ai cambiamenti
                              della variabile */
#define SHV_MODE_TYPE 0x70 /* Tipo della variabile. */
/* Tipi ad un byte */
#define SHV_MODE_TYPE_BOOL 0x00 /* Tipo booleano. */
#define SHV_MODE_TYPE_CHAR 0x10 /* Tipo signed char. */
#define SHV_MODE_TYPE_UCHAR 0x20 /* Tipo unsigned char. */
/* Tipi a quattro byte */
#define SHV_MODE_TYPE_LONG 0x30 /* Tipo long. */
#define SHV_MODE_TYPE_ULONG 0x40 /* Tipo unsigned long. */
#define SHV_MODE_TYPE_FLOAT 0x50 /* Tipo float. */
/* Tipi a otto byte */
#define SHV_MODE_TYPE_DOUBLE 0x60 /* Tipo double. */
/* Estensione : tipi con numero arbitrario di byte. */
#define SHV_MODE_TYPE_EXTENDED 0x70 /* Cosa sono ? */
/* Flag di variabile in RAM non volatile. La sua base non
 sara` "pchMem", ma "NVRAM_pub". */
#define SHV_MODE_RETAIN 0x80
/* Flag di variabile da azzerare al GO. */
#define SHV_MODE_RESET 0x100



enum ENUM_CMDS {
	CMD_ERRS,
	CMD_INFO,
	CMD_GO,
	CMD_HALT,
	CMD_LIST,
	CMD_MEMCHK,
	CMD_MEMCOPY,
	CMD_SET,
	CMD_STAT,
	CMD_STATUS,
	CMD_TRACE_A,
	CMD_TRACE_C,
	CMD_TRACE_D,
	CMD_TRACE_E,
	CMD_TRACE_M,
	CMD_TRACE_T,
	CMD_TRACE_V,

	CMD_TRIGGER_A,
	CMD_TRIGGER_C,
	CMD_TRIGGER_D,

	CMD_VAR,
	CMD_VER,
	CMD_WADD,
	CMD_WCREATE,
	CMD_WDEL,
	CMD_WENABLE,
	CMD_WRESET,
	CMD_WSHOW,
	CMD_NOTHING,

	CMD_UNKNOWN,
};

enum ENUM_ANSWRS {
	ANSWR_OK,
	ANSWR_A,
	ANSWR_E,
	ANSWR_D,
	ANSWR_L,

	ANSWR_UNKNOWN,
};

enum ENUM_SYMBOL_TYPES {
	VAR_NORMAL,
	VAR_SIGNAL,
};

enum ENUM_VAR_TYPES {
	TYPE_BOOL,
	TYPE_BYTE,
	TYPE_INT,
	TYPE_SINT,
	TYPE_DINT,
	TYPE_USINT,
	TYPE_UINT,
	TYPE_UDINT,
	TYPE_REAL,
	TYPE_TIME,
	TYPE_STRING,
	TYPE_WORD,
	TYPE_DWORD,

	TYPE_UNKNOWN,
};

/*
* plc types structure definition
*/
typedef struct TagPlcTypes{ 
	char *lpszName;	/* nome del tipo */
	unsigned int nLen;	  /* lunghezza del tipo */
} PLC_TYPES, *PPLC_TYPES; 

typedef union TagValue {
	/* byte nudi e crudi ... */
	unsigned char Bytes[8];
	unsigned int  BOOLValue;
	unsigned char BYTEValue;
	unsigned short WORDValue;
	unsigned long DWORDValue;
	double DoubleValue;
} VAR_VALUE;


/* 
* Plc Variable Structure Definition
*/
typedef struct TagPLCVar { 
	char szName[128];             /* Nome della variabile */
	char szDescription[128];      /* Descrizione della variabile */
	char szFile[128];             /* modulo in cui compare la prima referenza		SN */
	VAR_VALUE Value;              /* Valore della variabile (dato reale) */
	VAR_VALUE OldValue;           /* Vecchio alore della variabile (dato reale) */
	unsigned short bEnabled;      /* Variabile Identificata/certificata dalla MemChk */
	unsigned short nIndex1;       /* primo indice (se si tratta di un array) */
	unsigned short nIndex2;       /* secondo indice (se si tratta di un array) */
	unsigned long ulAddress;      /* indirizzo della variabile */
	unsigned int usType;          /* Tipo variabile */
	unsigned short usSymbolType;  /* tipo variabile (simbolo normale SN, segnale di scambio SS ...) */
	unsigned long ulDim1;         /* prima dimensione variabile (se array) */
	unsigned long ulDim2;         /* seconda dimensione variabile (se array) */
	unsigned long ulFlags;        /* flags specifici del segnale di scambio	*/
	unsigned long ulSize;         /* dimensione della variabile	*/
	unsigned long ulKey;          /* chiave dell'elemento */
} PLCVAR, *PPLCVAR; 

unsigned int CheckPLCResponse(int fd,unsigned int bReceive);
char *GetPLCVersion(char *szData);
BOOL GetPLCStatus(char *szData);
BOOL PLCConnect(int *fdPort,char *szHostName);
int OpenSocket(int *fdPort,char *name,int nPortNumber);
void GetMemChkInfo(PLCVAR *PLCVars,char *szData);
void SendVarListMemChk(int fd,PLCVAR *PLCVars);
int GetVarType(unsigned long ulFlags);
unsigned int GetVariableInfo(char *szData,PLCVAR *pPLCVar);
void GetVarsInfo(int fd,PLCVAR *PLCVars);
int ReceiveData(int fd,char *szRcvBuf);
unsigned int SendCmd(int fd,int nCmd,char *szParams,unsigned int bRecData);
int GetDebugValue(PLCVAR *pPV,char *pszBuffer,int nOffset);
void GetMemCopyInfo(PLCVAR *PLCVars,char *szData);

extern char *szCmds[];
extern char *szAnswrs[];
extern char *szErrs[];
extern PLC_TYPES PLCTypes[];
extern int nLastCommand;
extern char szRcvBuffer[MAX_RCV_SIZE];
