/*
* ca_file.h
* include per gestione configuration files
* rm : varie modifiche per de-izzarlo
*/

#ifndef _ca_file_
#define _ca_file_


#ifndef UINT_TYPE_DEFINED
typedef unsigned int UINT;
#define UINT_TYPE_DEFINED
#endif

#ifndef BOOL_TYPE_DEFINED
typedef unsigned int BOOL;
#define FALSE    0
#define TRUE     1
#define BOOL_TYPE_DEFINED
#endif

#ifndef MIN_MAX_DEFINED
#define max(a,b)    ((a)>=(b)?(a):(b))
#define min(a,b)    ((a)<=(b)?(a):(b))
#endif

#define True     1
#define False 0

#define L_F                    0x0A
#define C_R                    0x0D

#define IDS_XNC_BACKUP        ".bak"


/*
 *    ****************************************************************************
 *    PROTOTIPI
 *    ****************************************************************************
 */


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




#endif
