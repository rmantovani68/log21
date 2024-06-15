/**
* @file ep-common.h
* Include per funzioni EasyPicking
*
* @date Fri May 31 17:33:07 CEST 2002
*/

#pragma once
#include <ep-compatibility.h>

/* Lengths of message structure fields */
#define ADR_LEN      2
#define ACK_LEN      1
#define STAT_LEN     2
#define CLASS_LEN    2
#define CODE_LEN     2
#define SER_LEN      4
#define HDR_LEN      (2 * ADR_LEN + STAT_LEN + ACK_LEN + CLASS_LEN + CODE_LEN + SER_LEN)
#define INFO_LEN     4096    /* max. possible length */

/* Message class */
#define SPECIFIC_CLASS  1       /* specific message */
#define STRING_CLASS    2       /* message contains string */
#define SHARED_CLASS    3       /* message contains shared mem. handle */
#define FILE_CLASS      4       /* message contains file name */

/* Structure of a message, as it is sent */
typedef struct _ep_msg_t {
    char dest[ADR_LEN];         /* Module to which message is to be sent */
    char srce[ADR_LEN];         /* Module from which message originated */
    char ack[ACK_LEN];          /* =0: no confirm required, !=0, yes */
    char stat[STAT_LEN];        /* Status of module sending message */
    char class[CLASS_LEN];      /* Class of message (SPECIFIC_CLASS etc.) */
    char code[CODE_LEN];        /* Code of message */
    char serial[SER_LEN];       /* Serial no. of message */
    char info[INFO_LEN];        /* message information (not all sent) */
} ep_msg_t;

/* Structure for packing/unpacking of message header */
typedef struct _ep_msg_header_t{
    int dest;                   /* Module to which message is to be sent */
    int srce;                   /* Module from which message originated */
    int ack;                    /* =0: no confirm required, !=0, yes */
    int stat;                   /* Status of module sending message */
    int class;                  /* Class of message (SPECIFIC_CLASS etc.) */
    int code;                   /* Code of message */
    int serial;                 /* Serial no. of message */
} ep_msg_header_t;

#ifndef UINT_TYPE_DEFINED
    typedef unsigned int UINT;
    #define UINT_TYPE_DEFINED
#endif

typedef unsigned int ep_bool_t;

#ifndef FALSE
#define FALSE    (0)
#endif
#ifndef TRUE
#define TRUE     (1)
#endif

#ifndef MIN_MAX_DEFINED
    #define max(a,b)    ((a)>=(b)?(a):(b))
    #define min(a,b)    ((a)<=(b)?(a):(b))
    #define MIN_MAX_DEFINED
#endif

#ifndef BYTE_TYPE_DEFINED
    typedef unsigned char BYTE;
    #define BYTE_TYPE_DEFINED
#endif

#define CHAR_LF                    0x0A
#define CHAR_CR                    0x0D
#define BACK_EXTENSION    ".back"

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

typedef    struct    processo {
    char    szProcName[128]; /* nome file eseguibile */
    int     nQKey;           /* chiave della coda utilizzata */
    int        nQNumber;        /* numero della coda utilizzata */
    int        bCreate;         /* flag per creazione coda o sola apertura*/
    int        nGId;            /* numero processo*/
    int        nProcId;         /* pid del processo*/
    int        bKill;           /* 1 uccidere con kill, 0 inviare MSG_ENDMOD*/
    int        nPriority;       /* Priority Offset */
    int        bExecute;        /* 1 eseguire, 0 non eseguire */
    void   *zmq_context;
    void   *zmq_socket;
} PROCESSO;

typedef struct tagParseStruct{
    char *szName;
    char *szValue;
} PARSE_STRUCT, *PPARSE_STRUCT;

int ep_get_glib_default_flags(void);
void trace_debug(ep_bool_t bDate, ep_bool_t bNewLine, char *fmt,...)  __attribute__ ((format(printf,3,4)));

ep_bool_t PrintFile(char *szFile,char *szPrinter,char *szTitle,int nPrintType,ep_bool_t bDelete);

char *DeleteNewLine(char *str);
char *delete_new_line(char *str);
char *GetMonth(char *szString);
char *GetYear(char *szDateString);
char *GetDate(char *szDateString);
char *GetDateDDMM(char *szDateString);
char *GetDateDDMMYYYY(char *szDateString);
char *GetDateFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateFromYYYYMMDD(char *szFromDate,char *szDateString);
char *GetDateYYMMDD(char *szDateString);
char *GetDateYYMMFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateYYYYMMDD(char *szDateString);
char *GetDateYYYYMMDDFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDatef(char *szDateFormat);
char *GetTimeSpanHHMMSS(const char *szFromTime, const char* szToTime, char *szTimeString);
char *GetTime(char *szTimeString);
char *GetTimeFromHHMMSS(char *szFromTime,char *szTimeString);
char *GetTimeHHMM(char *szTimeString);
char *GetTimeHHMMSS(char *szTimeString);
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
ep_bool_t FileExists(char *szFileName);
ep_bool_t StrIsAlpha(char *str);
ep_bool_t StrIsDigit(char *str);
ep_bool_t StrIsAlnum(char *str);
ep_bool_t StrIsSpace(char *str);
ep_bool_t StrIsLower(char *str);
ep_bool_t StrIsUpper(char *str);
ep_bool_t StrIsXdigit(char *str);
ep_bool_t StrIsAscii(char *str);
char *Substitute(char *szString,char c1,char c2);
ep_bool_t RunSimpleProcess(int nProcID,char *szPath);
void TerminateProcess(int nProcID);
void ReadProcInfo(char *szFileConfig);
int PackMessage(char *msg, int dest, int srce, int ack, int stat, int class,int code, char *info, int info_len);
int UnpackMessage(char *msg, ep_msg_header_t *msghdr, char *info, int len);
ep_bool_t SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
int CreateProcessMsgQ(int nProcID, int clear);
int OpenProcessMsgQ(int nProcID);
void DeleteProcessMsgQ(int nProcID);
int create_zmq_context(void);
int destroy_zmq_context(void);

void  remove_parse_item(gpointer data, gpointer user_data);
GList *add_item_to_parse(GList **PSList,char *szName,char *szValue,ep_bool_t bCleanList);
char *ParseStatement(char *pszStatement,GList *PS);


void DetachSharedMemory(char *shm_ptr);
int CheckSharedMemory(int shm_key, int shm_size);
char *CreateSharedMemory(int shm_key, int shm_size);
char *OpenSharedMemory(int shm_key, int shm_size);
int DeleteSharedMemory(int shm_key, int shm_size);

char *ProcessGetName(int nProcID);
int ProcessReceiveMsgNoWait(int nProcID, int *mtype, char *msg);
int ProcessReceiveMsg(int nProcID, int *mtype, char *msg);

char    *GetFileBuffer    (char *, FILE *);
char    *GetBufferParagraph    (char *, char *, char *);
ep_bool_t GetBufferString    (char *, char *, char *, char *, int, char *);
int        GetBufferInt        (char *, char *, int, char *);
ep_bool_t GetFileString    (char *, char *, char *, char *, int, char *, FILE *);
int        GetFileInt        (char *, char *, int, char *, FILE *);
ep_bool_t PutFileString    (char *, char *, char *, char *, FILE *);
ep_bool_t PutFileInt        (char *, char *, int, char *, FILE *);
ep_bool_t DelFileString    (char *, char *, char *, FILE *);
ep_bool_t DelFileParagraph    (char *, char *, FILE *);
ep_bool_t PutFileBuffer    (char *, char *, long);
ep_bool_t BackupFile        (char *);

int text_read_file(char *szFileName,char *szBuffer);
int GetMaxLineWidth(char *szFile);


ep_bool_t ReadCfgItems(PCFGITEM pItems,char *pszCfgFileName,int nPID);
ep_bool_t WriteCfgItems(PCFGITEM pItems,char *pszCfgFileName);
ep_bool_t TraceCfgItems(PCFGITEM pItems);
ep_bool_t ReadCfg(gchar *szCfgFileName, PCFGITEM pCfgItems, ep_bool_t bReadProcInfo);

/*
* plcsock.h
* include file per plcsock.c
*/

#define PLC_SERVICE_PORT 8039

/* dimensione del buffer di ricezione */
#define MAX_RCV_SIZE    2048
/* numero max di variabili PLC */
#define MAX_PLC_VARS    1024

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
    char *lpszName;    /* nome del tipo */
    unsigned int nLen;      /* lunghezza del tipo */
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
    char szFile[128];             /* modulo in cui compare la prima referenza        SN */
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
    unsigned long ulFlags;        /* flags specifici del segnale di scambio    */
    unsigned long ulSize;         /* dimensione della variabile    */
    unsigned long ulKey;          /* chiave dell'elemento */
} PLCVAR, *PPLCVAR; 

unsigned int plc_check_response(unsigned int bReceive);
void plc_send_vars_list_mem_chk(PLCVAR *PLCVars);
int plc_get_var_type(unsigned long ulFlags);
unsigned int plc_get_var_info(char *szData,PLCVAR *pPLCVar);
void plc_get_vars_info(PLCVAR *PLCVars);
int plc_receive_data(char *szRcvBuf);
unsigned int plc_send_cmd(int nCmd,char *szParams,unsigned int bRecData);
int plc_get_debug_value(PLCVAR *pPV,char *pszBuffer,int nOffset);
void plc_refresh_vars(void);
void plc_update_var_values(unsigned char refresh);

int plc_write_vars_list(char *szCfgFile);
int plc_read_vars_list(char *szCfgFile);
ep_bool_t plc_up_front(int nVarIndex);
ep_bool_t plc_down_front(int nVarIndex);
ep_bool_t plc_set_var_value(int  nVarIndex, int nValue);
unsigned long plc_var_value(int  nVarIndex);
unsigned long plc_var_old_value(int  nVarIndex);
ep_bool_t plc_var_value_changed(int  nVarIndex);
int plc_get_var_index(char *szVarName);
char *plc_get_var_name(int nIndex);
ep_bool_t plc_connect(char *szHostName, int service_port);
int plc_get_status(void);
char *plc_get_version(void);

/* xdg */
char *xdg_user_dir_lookup (const char *type);

extern ep_bool_t __ep_debug_level__;
ep_bool_t ep_set_debug_level(ep_bool_t dl);
