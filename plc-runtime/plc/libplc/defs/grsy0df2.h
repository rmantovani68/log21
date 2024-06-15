/****************************************************************************
* File        : grsy0df2.h     
* Description : Common constants, macros and structure definitions
* Creation    : 04/03/92 LIS   
*****************************************************************************
* Modif       : 28/01/94 DEF_COMPACT
*             : 15/02/94 mod_symb
*             : 08/02/95 GG : Aggiunta la definizione di uno spazio destinato
*                             a contenere la tabella di conversione per i
*                             segnali di scambio.
*****************************************************************************/


#pragma pack(1)

/*************** Variables types ************/

#define BOO 1
#define ANA 2        
#define TMR 3 
#define MSG 4        
#define GRA 5        
#define REA 6
#define ETA 7        
#define TRA 8 

#define USP 11
#define LST 12

#define FBP 14  
#define ERR 15

/**************  Variables attributes *************/

#define ATTR_INT  0
#define ATTR_OUT  1
#define ATTR_INP  3

/************* ISaGRAF virtual address ***********/

#define NUMVAR(x) (word)((x) & 0x0fff)
#define TYPVAR(x) (word)((x)>>12)

/************* Memory space management **********/

#define SPC_SYS  0   /* system module */
#define SPC_APL  1   /* application module */
#define SPC_IOS  2   /* io module */
#define SPC_SYM  3   /* symbol module */
#define SPC_KER  4   /* kernel module */
#define SPC_TST  5   /* tester module */
#define SPC_USF  6   /* usf trap */
#define SPC_FBL  7   /* fbl trap */
#define SPC_MF1  8   /* modif 1 */
#define SPC_MF2  9   /* modif 2 */
#define SPC_TBL  10  /* Tabella di conversione per le applicazioni CNI. */
#define SPC_STB  11  /* Symbol table */
#define SPC_HTB  12  /* Hash table */

/**************** Kernel status **************/

#define RT 0          /* Real time */ 
#define CC 1          /* Cycle by cycle */
#define EC 2          /* Execute a cycle */
#define BK 4          /* Breakpoint */ 
#define RS 255        /* Reset */

/**************** Application status *********/

#define START_INIT 0  /* Initial status */
#define START_APL  1  /* Application ready */
#define START_OK   2  /* Finished */
#define START_STOP 3  /* Stop request */

/**************** System module *************************/

#define MAX_QUESTION 256 /* Dimensione massima di un messaggio del client. */

typedef struct 
{
   unsigned char start;             /* application status */
   unsigned char mode_ker;          /* kernel status */    
   unsigned char bkp;               /* breakpoint encountered */
   unsigned char reserve;    

   long tcy;                        /* authorised */    
   long tcy_current;                /* current */
   long tcy_max;                    /* maximum */
   long depass;                     /* overflow */    
   long count;                      /* number of cycles */

   unsigned char question_ready;    /* question to kernel */
   unsigned char client;            /* client semaphore */
   unsigned char *question_ker;     /* question to server */
   unsigned char *answer_ker;       /* response to kernel */
   short         answer_size;       /* response size */ 

   char mod_appli[32];              /* application module name */

   byte buffer[MAX_QUESTION];        /* question buffer */

} str_system;

#define MAX_INT 0x7fffffffL
#define MIN_INT 0x80000000L
#define MAX_REA 1.0E+38
#define MIN_REA -1.0E+38
#define MAX_TMR 8640000      /* 24h in 10ms units = 0x83d600 */

#ifdef DEF_COMPACT
#define SYST  ((str_system *)BASE_RAM)     /* base address system table */
#else
extern str_system *SYST;                   /* defined in main */
#endif


#pragma pack()

/* eof */
