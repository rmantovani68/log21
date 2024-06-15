/*
* mainfun.h
* Dichiarazione Funzioni Utilizzate
* Easy Picking 3.0
* Copyright A&L srl 1999-2000
* Autore : Roberto Mantovani
*/

/*
* mainutil.c
*/
char *GetTime(long time,char *szTimeString);
char *GetTimeFromHHMMSS(char *szFromTime,char *szTimeString);
char *GetTimeHHMMSS(long time,char *szTimeString);
char *GetTimeHHMM(long time,char *szTimeString);
char *GetDate(long time,char *szDateString);
char *GetDateYYYYMMDD(long time,char *szDateString);
char *GetDateYYMMDD(long time,char *szDateString);
char *GetDatef(char *szDateFormat);
char *GetDateDDMM(long time,char *szDateString);
char *GetDateDDMMYYYY(long time,char *szDateString);
char *GetDateFromYYYYMMDD(char *szFromDate,char *szDateString);
char *GetDateYYMMFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateYYYYMMDDFromDDMMYY(char *szFromDate,char *szDateString);
char *GetDateFromDDMMYY(char *szFromDate,char *szDateString);
BOOL FileExists(char *szFileName);
char *LeftStr(char *szString,int nLen);
char *SubStr(char *szString,int nPos,int nLen);
char *StrTrimAll(char *str);
char *StrTrimRight(char *str);
char *StrTrimLeft(char *str);
char *ElimNewLine(char *str);
char *RightPad(char *szString,int nLen);
char *PadZero(char *szString,int nLen);
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
int ContaRighe(char *szFileName);
void gtk_label_printf(GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,2,3)));
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,3,4)));
void gtk_text_read_file(GtkWidget *w,char *szFileName);
void gtk_update(void);
void gtk_widget_set_foreground(GtkWidget *w,GdkColor *Color);
void gtk_widget_set_background(GtkWidget *w,GdkColor *Color);
unsigned long CryptString(char *pszString);
BOOL RunSimpleProcess(int nProcID);
void TerminateProcess(int nProcID);
int CreateListFromSelect(GtkWidget *super_parent, GtkWidget *parent, GtkWidget **clist, char *szCListName, int nSelectMode, char *szSqlCmd);
int RefreshListFromSelect(GtkWidget *super_parent, GtkWidget *parent, GtkWidget *clist, char *szCListName, int nSelectMode, char *szSqlCmd);
int CreateTable(GtkWidget *parent,char *szScrolledWindowName,char *szCListName,char *szLabelTitle,char *szLabelDescr,char *szLabelRows,char *szParagraph,char *szTableName,int nSelectionMode,char *szKey);
int RefreshTable(GtkWidget *parent,char *szScrolledWindowName,char *szCListName,char *szLabelDescr,char *szLabelTitle,char *szLabelRows,char *szParagraph,char *szTableName,int nSelectionMode,char *szKey);
void trace_debug(GdkColor *Color,...);
BOOL PrintTable(char *szParagraph,char *szTableName,char *szPrinterName,char *szKey,FILE *fp);
void PrintFile(char *szFile,char *szPrinter,char *szTitle,BOOL bA2PS,int nWidth);
int PrintListFromSelect(FILE *fp,char *szSqlCmd);
int GetMaxLineWidth(char *szFile);


/*
* mainfun.c
*/
void gtk_error_handler(gchar *str);
void gtk_warning_handler(gchar *str);
void gtk_message_handler(gchar *str);
void ReadConfiguration(BOOL bReadProcInfo);
void ExitApp();
int UpdateListFromSelect(GtkWidget *clist, char *szSqlCmd, char *szKey, int nKeyField);
void EditConfigurazione(void);
void ApplyConfigurazione(GtkWidget *dlg);
void load_table(char *pszTableName,char *szKey);
void edit_dlg_ordini     (GtkWidget *dlg, char *pszCodiceOrdine);
void edit_dlg_opeset     (GtkWidget *dlg, char *pszCodice);
void edit_dlg_prodotto   (GtkWidget *dlg, char *pszCodProd);
void edit_dlg_imballi    (GtkWidget *dlg, char *pszCodiceImballo);
void edit_dlg_ubicazioni (GtkWidget *dlg, char *szCodUbi);
void edit_dlg_operatori  (GtkWidget *dlg, char *pszCodiceOperatore);
void edit_dlg_collo      (GtkWidget *dlg, char *pszOrdine, int nCollo);
char *get_ubicazione_in_edit(GtkWidget *dlg, char *pszUbicazione);
char *GetStatoOrdine(char cStato,char *pszBuffer);
int GetKeyIndex(char *pszKey,GtkCList *clist,int nColumn, int nStartRow);
int search_key(char *pszKey,GtkCList *clist,int nColumn,int nStartRow,BOOL bCaseSensitive);
void clist_update_selected_row(GtkCList *clist, int nStep);
void clist_move_to_row(GtkCList *clist, int nRow);
GtkWidget *CreateDBEditDlg(char *szDBName);
BOOL UpdateDBEditDlg(GtkWidget *dlg,char *szDBName,char *szKey);
BOOL GetKeysList(char *szDBName,GtkCList *clist,int nRow,char *pszKey);

/*
* main.c
*/
int main (int argc, char *argv[]);
