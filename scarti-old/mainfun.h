/*
* mainfun.h
* Dichiarazione Funzioni Utilizzate
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

/*
* mainutil.c
*/
void ChangeBackgroundColor(GtkWidget *w,GdkColor *Color);
void ChangeTextColor(GtkWidget *w,GdkColor *Color);
int CreateListFromSelect(GtkWidget *super_parent, GtkWidget *parent, GtkWidget **clist, char *szCListName, int nSelectMode, char *szSqlCmd,char *szKey);
unsigned long CryptString(char *pszString);
char *DeleteNewLine(char *str);
BOOL FileExists(char *szFileName);
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
char *GetTime(long time,char *szTimeString);
char *GetTimeFromHHMMSS(char *szFromTime,char *szTimeString);
char *GetTimeHHMM(long time,char *szTimeString);
char *GetTimeHHMMSS(long time,char *szTimeString);
char *LeftStr(char *szString,int nLen);
char *PadZero(char *szString,int nLen);
void PrintFile(char *szFile,char *szPrinter,char *szTitle,BOOL bA2PS,int nWidth);
int PrintListFromSelect(FILE *fp,char *szSqlCmd, char *szOrdProg, int nNumCollo);
BOOL PrintTable(char *szParagraph,char *szTableName,char *szPrinterName,char *szKey);
int RefreshTable(GtkWidget *parent,char *szScrolledWindowName,char *szCListName,char *szLabelName,char *szParagraph,char *szTableName,int nSelectionMode,char *szKey);
char *RightPad(char *szString,int nLen);
char *RightStr(char *szString,int nLen);
BOOL RunSimpleProcess(int nProcID);
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
char *StrTrimAll(char *str);
char *StrTrimLeft(char *str);
char *StrTrimRight(char *str);
char *SubStr(char *szString,int nPos,int nLen);
void TerminateProcess(int nProcID);
int UpdateListFromSelect(GtkWidget *clist, char *szSqlCmd, char *szKey, int nKeyField);
int UpdateTable(GtkWidget *parent,char *szCListName,char *szParagraph,char *szTableName,char *szKey);
void gtk_label_printf(GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,2,3)));
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,3,4)));
void gtk_update(void);
int GetMaxLineWidth(char *szFile);
int gtk_clist_search_key(char *pszKey,GtkCList *clist,int nColumn,int nStartRow,BOOL bCaseSensitive);
void gtk_clist_move_to_row(GtkCList *clist, int nRow);

/*
* mainfun.c
*/
void ApplyConfigurazione(GtkWidget *dlg);
void EditConfigurazione(GtkWidget *dlg);
void ExitApp();
BOOL ReadConfiguration(void);
void ReadProcInfo(char *szFileConfig);
void gtk_error_handler(gchar *str);
void gtk_message_handler(gchar *str);
void gtk_warning_handler(gchar *str);
void trace_debug(GdkColor *Color,GtkWidget *txt,...);
GdkColor *GetColorValue(PCOLORSTRUCT pColors,int nColorIndex);
int GetColorIndex(PCOLORSTRUCT pColors,char *szColorName);

/*
* mainproc.c
*/
gint ProcessMsgs( gpointer data );

/*
* peso.c
*/
int main (int argc, char *argv[]);
