/*
* ep-db.h
* ----------
* Include per funzioni EasyPicking
*
* Data creazione : Fri May 31 17:33:07 CEST 2002
*/

#ifndef _ep_db_h_
#define _ep_db_h_

typedef void DBconn;
typedef void DBresult;

typedef enum
{
	DBRES_EMPTY_QUERY = 0,
	DBRES_COMMAND_OK,
	DBRES_TUPLES_OK,
	DBRES_COPY_OUT,
	DBRES_COPY_IN,
	DBRES_BAD_RESPONSE,
	DBRES_NONFATAL_ERROR,
	DBRES_FATAL_ERROR
} DBExecStatusType;

typedef enum
{
	DB_TYPE_NONE,
	DB_TYPE_STRING,
	DB_TYPE_INTEGER,
	DB_TYPE_FLOAT,
	DB_TYPE_DATE,
	DB_TYPE_TIME,
	DB_TYPE_DATETIME,
	DB_TYPE_TIMESTAMP,
	DB_TYPE_YEAR
} DBFieldType;

typedef struct _DbType {
	int type_id;
	char *type_name;
	DBFieldType type;
} DbType;

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

#define MAX_FIELDS_NUMBER	256
#define MAX_TAGS_NUMBER		 64
#define FIELD_TITLE_LEN    40
#define FIELD_DESCR_LEN    80

typedef struct tagTag {
	char szTagName[80];                      /* Tag Name */
	char szExpression[128];                  /* Expression */
	int  nUnique;                            /* Unique */
} TAG, *PTAG;

typedef struct tagField {
	char szFieldName[20];                    /* Field Name */
	char szFieldTitle[FIELD_TITLE_LEN];      /* Field Title */
	char szFieldDescr[FIELD_DESCR_LEN];      /* Field Description */
	int  nFieldLen;                          /* Field Len */
	int  nFieldDec;                          /* Field Dec */
	char cFieldType;                         /* Field Type */
	unsigned int nLineNumber;               /* Line Number */
	unsigned int nFieldOffset;              /* Field Offset */
	unsigned char nFieldFlag;               /* Field Flag */
	unsigned int nDisplayLength;            /* Display Lenght*/
} FIELD, *PFIELD;

typedef struct tagDBStruct {
	char szDBName[80];                            /* Nome della base di dati */
	char szTableName[80];                         /* Nome della Tabella */
	PFIELD pFields[MAX_FIELDS_NUMBER];            /* puntatore all'array dei campi */
	TAG Tags[MAX_TAGS_NUMBER];	                  /* puntatore all'array dei tags (indici) */
	int nFieldsNumber;                            /* Numero dei campi contenuti in struttura */
	int nTagsNumber;                              /* Numero dei tags contenuti nell'indice */
	int nActualTag;                               /* indice corrente */
	int nRecordLen;                               /* Lunghezza Record ASCII */
	char szRecordType[80];                        /* Codice Tipo Record */
	int nRecordTypePos;                           /* Posizione Tipo Record */
	int nRecordTypeLen;                           /* Lunghezza Tipo Record */
	int nDataStart;                               /* Start of Data */
	int nLinesNumber;                             /* Number of Lines (multiline records) */
	int nLineNumberPos;                           /* Line Number Position (file ascii)*/
	int nTableMask;                               /* Table Bitmaks (see top of file) */
} DBSTRUCT, *PDBSTRUCT;

typedef struct {
	DBSTRUCT *pDB;
	char szCfgPath[128];
	char szCfgName[128];
} DB_ARRAY_ITEM_STRUCT, *PDB_ARRAY_ITEM_STRUCT;

char *NormalizeString(char *szString);
char *NormalizeFormat(char *szString);
BOOL ReadTableInfo(PDBSTRUCT pDB,char *szPath,char *szCfgBaseName);
void ReadTablesInfo(void);
void InsertTableInfo(PDBSTRUCT pDB,char *szCfgPath,char *szCfgName);
void FreeTablesInfo(void);
BOOL IsAsciiRecord(char *szBuffer,PDBSTRUCT pDB);
BOOL GetAsciiRecord(char *szBuffer,PDBSTRUCT pDB);
int UpdateAsciiRecord(char *szBuffer, PDBSTRUCT pDB, BOOL bUpdate);
BOOL WriteAsciiRecord(FILE *fp,PDBSTRUCT pDB,DBresult *DBRes,int nIndex,BOOL bFillZero,BOOL bSigned);
int ReadFieldsInfo(char *szCfgFile,PDBSTRUCT pDB);
int ReadTagsInfo(char *szCfgFile,PDBSTRUCT pDB);
BOOL DBTestConnection(BOOL bReset);
BOOL DBConnect(char *szHost,char *szPort,char *szDataBase);
BOOL DBConnectUser(char *szHost,char *szPort,char *szDataBase,char *szUserName,char *szPassword);
BOOL DBReconnectUser(char *szHost,char *szPort,char *szDataBase,char *szUserName,char *szPassword);
BOOL DBDisconnect(void);
BOOL DBConnectionActive(void);
DBresult *DBExecQuery(BOOL bTrace,char *format, ...) __attribute__ ((format(printf,2,3)));
DBExecStatusType DBresultStatus(DBresult *DBRes);
char *DBcmdTuples(DBresult *DBRes);
void DBFreeQueryResult(DBresult *DBRes);
int DBntuples(DBresult *DBRes);
int DBnfields(DBresult *DBRes);
unsigned int DBgetlength(DBresult *DBRes,int nTuple,int nField);
char *DBfname(DBresult *DBRes,int nField);
int DBfnumber(DBresult *DBRes,char *szFieldName);
int DBftype(DBresult *DBRes,int nField);
char *DBgetvalue(DBresult *DBRes,int nTuple,int nField);
char *DBresultErrorMessage(DBresult *DBRes);
DBFieldType db_get_field_type(int nType);
int PrintListFromSelect(FILE *fp,char *szSqlCmd, char *szOrdProg, int nNumCollo);
BOOL PrintTable(char *szCfgFileName,char *szParagraph,char *szTableName,char *szPrinterName,char *szPathStampe,GList *PSList,FILE *fp,int nPrintType);
BOOL WriteTable(char *szCfgFileName,char *szParagraph,char *szTableName,char *szFileName,GList *PSList);

#ifdef USE_POSTGRES
#include <ep_pgsql.h>
#endif

#endif /* _ep_db_h_ */

