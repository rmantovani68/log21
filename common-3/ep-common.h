/*
* ep-common.h
* ----------
* Include per funzioni EasyPicking
*
* Data creazione : Fri May 31 17:33:07 CEST 2002
*/

#ifndef _ep_common_h_
#define _ep_common_h_

#ifndef UINT_TYPE_DEFINED
	typedef unsigned int UINT;
	#define UINT_TYPE_DEFINED
#endif

#ifndef BOOL_TYPE_DEFINED
	typedef unsigned int BOOL;

	#define	FALSE	(0)
	#define	TRUE	(!FALSE)
	#define BOOL_TYPE_DEFINED
#endif

#ifndef MIN_MAX_DEFINED
	#define max(a,b)	((a)>=(b)?(a):(b))
	#define min(a,b)	((a)<=(b)?(a):(b))
	#define MIN_MAX_DEFINED
#endif


#ifndef BYTE_TYPE_DEFINED
	typedef unsigned char BYTE;
	#define BYTE_TYPE_DEFINED
#endif

#define CHAR_LF					0x0A
#define CHAR_CR					0x0D
#define BACK_EXTENSION	".back"

typedef enum {
	CFG_TYPE_NONE,
	CFG_TYPE_BOOL,
	CFG_TYPE_INT,
	CFG_TYPE_STRING
} CFG_ITEM_TYPE;

typedef enum {
	CFG_NO_WIDGET,
	CFG_SPIN_BUTTON_WIDGET,
	CFG_ENTRY_WIDGET,
	CFG_TOGGLE_BUTTON_WIDGET,
} CFG_WIDGET_TYPE;

typedef union tagCfgValue {
	unsigned int *pbBool;
	int  *pnInt;
	char *pszString;
} CFGVALUE, *PCFGVALUE;

typedef union tagCfgDefaultValue {
	unsigned int bBool;
	int  nInt;
	char *szString;
} CFGDEFAULTVALUE, *PCFGDEFAULTVALUE;

typedef struct tagCfgItem {
	char *pszParagraph;
	char *pszItemName;
	CFG_ITEM_TYPE ItemType;
	CFGVALUE ItemValue;
	CFGDEFAULTVALUE ItemDefaultValue;
	CFG_WIDGET_TYPE WidgetType;
	char *pszWidgetName;
} CFGITEM, *PCFGITEM;

typedef struct tagCfgArray{
	CFGITEM *Items;
} CFGARRAY, *PCFGARRAY;

typedef	struct	processo {
	char	szProcName[128]; /* nome file eseguibile */
	int     nQKey;           /* chiave della coda utilizzata */
	int		nQNumber;        /* numero della coda utilizzata */
	int		bCreate;         /* flag per creazione coda o sola apertura*/
	int		nGId;            /* numero processo*/
	int		nProcId;         /* pid del processo*/
	int		bKill;           /* 1 uccidere con kill, 0 inviare MSG_ENDMOD*/
	int		nPriority;       /* Priority Offset */
	int		bExecute;        /* 1 eseguire, 0 non eseguire */
} PROCESSO;

typedef struct tagParseStruct{
	char *szName;
	char *szValue;
} PARSE_STRUCT, *PPARSE_STRUCT;


BOOL PrintFile(char *szFile,char *szPrinter,char *szTitle,int nPrintType,BOOL bDelete);
char *Escape(char *t);
char *Unescape(char *t);

char *DeleteNewLine(char *str);
char *ElimNewLine(char *str);
char *GetYear(long time,char *szDateString);
char *GetDate(long time,char *szDateString);
char *GetDateDDMM(long time,char *szDateString);
char *GetDateDDMMYYYY(long time,char *szDateString);
char *GetDateFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateFromYYYYMMDD(char *szFromDate,char *szDateString);
char *GetDateYYMMDD(long time,char *szDateString);
char *GetDateYYMMFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateYYYYMMDD(long time,char *szDateString);
char *GetDateYYYYMMDDFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDatef(char *szDateFormat);
char *GetTimeSpanHHMMSS(const char *szFromTime, const char* szToTime, char *szTimeString);
char *GetTime(long time,char *szTimeString);
char *GetTimeFromHHMMSS(char *szFromTime,char *szTimeString);
char *GetTimeHHMM(long time,char *szTimeString);
char *GetTimeHHMMSS(long time,char *szTimeString);
char *GetDateYYYYMMDDFromDD_MM_YYYY(char *szFromDate,char *szDateString);
char *GetDateYYYYMMDDFromYYYY_MM_DD(char *szFromDate,char *szDateString);
char *LeftStr(char *szString,int nLen);
char *PadZero(char *szString,int nLen);
char *RightPad(char *szString,int nLen);
char *LeftPad(char *szString,int nLen);
char *RightStr(char *szString,int nLen);
char *StrTrimAll(char *str);
char *StrTrimLeft(char *str);
char *StrTrimRight(char *str);
char *StrNormalize(char *str);
char *SubStr(char *szString,int nPos,int nLen);
char *i_strtrimall(char *str);
int CmpFileDate(char *pszFileName);
char *GetFileDateTime(char *pszFileName);
BOOL FileExists(char *szFileName);
BOOL StrIsAlpha(char *str);
BOOL StrIsDigit(char *str);
BOOL StrIsAlnum(char *str);
BOOL StrIsSpace(char *str);
BOOL StrIsLower(char *str);
BOOL StrIsUpper(char *str);
BOOL StrIsXdigit(char *str);
BOOL StrIsAscii(char *str);
char *Substitute(char *szString,char c1,char c2);
BOOL RunSimpleProcess(int nProcID,char *szPath);
void TerminateProcess(int nProcID);
void ReadProcInfo(char *szFileConfig);
BOOL SendMessageNoWait(int nDest,int nSrce,int nMsgCode,char *szText,int nTimeOut,int nRetries);
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
int CreateProcessMsgQ(int nProcID, int clear);
int OpenProcessMsgQ(int nProcID);
void DeleteProcessMsgQ(int nProcID);

void  remove_parse_item(gpointer data, gpointer user_data);
GList *add_item_to_parse(GList **PSList,char *szName,char *szValue,BOOL bCleanList);
char *ParseStatement(char *pszStatement,GList *PS);


void DetachSharedMemory(char *shm_ptr);
int CheckSharedMemory(int shm_key, int shm_size);
char *CreateSharedMemory(int shm_key, int shm_size);
char *OpenSharedMemory(int shm_key, int shm_size);
int DeleteSharedMemory(int shm_key, int shm_size);

char *ProcessGetName(int nProcID);
int ProcessReceiveMsgNoWait(int nProcID, int *mtype, char *msg);
int ProcessReceiveMsg(int nProcID, int *mtype, char *msg);

char	*GetFileBuffer	(char *, FILE *);
char	*GetBufferParagraph	(char *, char *, char *);
BOOL	GetBufferString	(char *, char *, char *, char *, int, char *);
int		GetBufferInt		(char *, char *, int, char *);
BOOL	GetFileString	(char *, char *, char *, char *, int, char *, FILE *);
int		GetFileInt		(char *, char *, int, char *, FILE *);
BOOL	PutFileString	(char *, char *, char *, char *, FILE *);
BOOL	PutFileInt		(char *, char *, int, char *, FILE *);
BOOL	DelFileString	(char *, char *, char *, FILE *);
BOOL	DelFileParagraph	(char *, char *, FILE *);
BOOL	PutFileBuffer	(char *, char *, long);
BOOL	BackupFile		(char *);

int text_read_file(char *szFileName,char *szBuffer);
int GetMaxLineWidth(char *szFile);


BOOL ReadCfgItems(PCFGITEM pItems,char *pszCfgFileName,int nPID);
BOOL WriteCfgItems(PCFGITEM pItems,char *pszCfgFileName);
BOOL TraceCfgItems(PCFGITEM pItems);
BOOL ReadCfg(gchar *szCfgFileName, PCFGITEM pCfgItems, BOOL bReadProcInfo);

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

/* dimensione del buffer di ricezione */
#define MAX_RCV_SIZE	2048
/* numero max di variabili PLC */
#define MAX_PLC_VARS	1024

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

unsigned int CheckPLCResponse(unsigned int bReceive);
char *GetPLCVersion(char *szData);
BOOL GetPLCStatus(char *szData);
int OpenSocket(int *fdPort,char *name,int nPortNumber);
void GetMemChkInfo(PLCVAR *PLCVars,char *szData);
void SendVarListMemChk(PLCVAR *PLCVars);
int GetVarType(unsigned long ulFlags);
unsigned int GetVariableInfo(char *szData,PLCVAR *pPLCVar);
void GetVarsInfo(PLCVAR *PLCVars);
int ReceiveData(char *szRcvBuf);
unsigned int SendCmd(int nCmd,char *szParams,unsigned int bRecData);
int GetDebugValue(PLCVAR *pPV,char *pszBuffer,int nOffset);
void GetMemCopyInfo(char *szData);

int WriteVarList(char *szCfgFile);
int ReadVarList(char *szCfgFile);
BOOL UpFront(int nVarIndex);
BOOL DownFront(int nVarIndex);
BOOL SetVarValue(int  nVarIndex, int nValue);
unsigned long VarValue(int  nVarIndex);
unsigned long VarOldValue(int  nVarIndex);
BOOL VarValueChanged(int  nVarIndex);
int GetVarIndex(char *szVarName);
BOOL PLCConnect(char *szHostName);

extern char *szCmds[];
extern char *szAnswrs[];
extern char *szErrs[];
extern PLC_TYPES PLCTypes[];
extern PLCVAR PLCVars[MAX_PLC_VARS];
extern int nLastCommand;
extern char szRcvBuffer[MAX_RCV_SIZE];
extern int fdPLC;
#endif /* _ep_common_h_ */
