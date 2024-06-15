/****************************************************************************
* Fichier     : grsy0ker.h
* Description : Real time kernel data base
* Creation    : LIS 28/02/93
*****************************************************************************
* Modif       : 15/11/93 str_msg 
*             : 19/01/94 td_fbl
*             : 28/01/94 DEF_COMPACT
****************************************************************************/

/********* Kernel procedure prototypes *******/

#include <grke0pro.h>


#pragma pack(1)

/********** Grafcet status ********************/

#define STS_INACTIF 0x00      
#define STS_ACTIF   0x02      /* active */
#define STS_FIGE    0x04      /* frozen */

/********** Execution context for output refresh *****/

#define TPROG_NUL 0  /* any */
#define TPROG_CMB 1  /* combinatory */
#define TPROG_SFC 2  /* SFC */ 
#define TPROG_SRV 3  /* Server */
#define TPROG_EST 4  /* end of step */
#define TPROG_BST 5  /* beginning of step */
#define TPROG_RST 6  /* repeat action of step */

/********** CJ Run time objects **************************/

typedef struct str_msg    
{    
   char buffer[256]; 
   struct str_msg *next;  
} str_msg;

typedef struct str_bkp
{    
    word breakmic;           
    struct str_bkp *next;
    word *adresse;           
    word va;                 
    word microcode;          
    byte type;               
    byte rencontre;          
} str_bkp;

typedef struct str_action
{
   struct str_action *next;         
   word *sequence;              /* Debut sequence */
   word step;                   /* Etape origine */
   word numgra;                 /* Numero grafcet origine */
} str_action;

/********* Offset table **********************/

typedef struct str_ker
{                                             
     /********* Pointers **************/

     byte **ad_msg;    /* table adresses messages */

     word **seq_recep; /* table des pointeurs sequences receptivite */
     word **stp_amont; /* table des pointeurs etapes amont */
     word **stp_aval;  /* table des pointeurs etapes avales */

     word **seq_debut; /* table des pointeurs actions de debut */
     word **seq_comp;  /* table des pointeurs actions complexes */
     word **seq_fin;   /* table des pointeurs actions de fin */
     word **trs_aval;  /* table des pointeurs transitions avales */

     /********* Byte array **************/

     byte *ac_tmr;      /* activite tmr */

     /********* Bits arrays **************/

     byte *ac_stp;      /* activite etape */
     byte *mk_trs;      /* marquage transition  */
     byte *mk_stp;      /* marquage etape traitee */
     byte *reserv1;  
     byte *reserv2; 
     byte *reserv3;

     /********* Byte arrays **************/

     byte *bf_boo;      /* buffer boo */
     byte *st_gra;      /* statuts grafcets */

     /********* Long arrays *************/

     long *bf_ana;     /* buffer ana */ 
     long *bf_tmr;     /* buffer tmr */
     long *st_tim;     /* step duration */
     word **base_gra;  /* table des pointeurs de code grafcet */

     /********* Byte lists **************/

     byte *ex_gra;      /* grafcet a examiner */

     /********* Word lists *****************/

     word *ex_lrep;     /* etapes repetitives last cycle */     
     word *ex_nrep;     /* etapes repetitives new cycle */  
     word *fr_trs;      /* transitions franchissables */

     word *rf_boo;      /* refresh boo output */
     word *rf_ana;      /* refresh ana output */
     word *rf_msg;      /* refresh msg output */

     /********* Pointer lists ***********/

     word *ex_trs; /* transitions a examiner ou valides */

     /********* Object lists **************/

     byte *bf_msg; 
     byte *td_act;        /* liste actions */
     str_msg *lst_msg;    /* liste blocs message */
     str_bkp *lst_bkp;    /* liste points d'arret */
     long *td_stack;      /* pile */
     byte *td_fbl;        /* memory for fbl */

} str_ker;                                                         

#ifdef DEF_COMPACT
#define KER ((str_ker *)BASE_RAM+sizeof(str_system)) 
#else
extern str_ker *KER;         /* kernel table */ 
#endif


#pragma pack()

/* eof */


