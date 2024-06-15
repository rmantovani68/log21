/***************************************************************************
* File        : grio0def.h
* Description : IO structures 
* Creation    : 03/02/93  LIS
****************************************************************************
* Modif       : 10/01/94 iobrd->user2 
****************************************************************************/

#pragma pack(1)

/************ workstation variable definition (read only) **************/

typedef struct 
{                    
  word va;           /* Isagraf virtual address */
  word usadd;        /* User address */ 
  byte num;          /* Board number */
  byte channel;      /* Channel number on board */
  byte out_real;     /* 80:out 4:real */ 
  byte conv_len;     /* Conversion number or message length */   
} str_dfvar;

#define IO_REAL 0x04 /* Real analog */
#define IO_OUT  0x80 /* Output */

/********* workstation board definition (read only) ********************/

typedef struct
{
   word total_size;  /* Complete size of this structure */
   word oem_key;     /* OEM key code */
   byte num;         /* Board or slot number */
   byte real;        /* 1 = real, 0 = simulation board */
   byte type;        /* 1 = boo, 2 = ana, 4 = msg */
   byte output;      /* 0 = input, 1 = output */
   byte nbchannel;   /* Maximum number of variables */
   byte nbvar;       /* Number of connected variables */ 
   word oem_size;    /* Size of oem info of this structure */
                     /* Free space for oem parameters */
} str_dfbrd;

/********* run time variable object ***********************/

typedef struct 
{                    
  struct str_iobrd *iobrd; /* Pointer to board object          (read only) */  
  str_dfvar *dfvar;        /* Pointer to variable definition   (read only) */ 
  void *data;              /* Pointer to value buffer          (read only) */
  void (*oem_call)();      /* Variable management procedure    (read/write) */

  word vnum;               /* Isagraf variable number          (read only) */

  byte channel;            /* Channel number on board          (read only) */
  byte cbl;                /* Conv., bit number or lenght(MSG) (read only) */   
  byte lock;               /* Lock = 1, normal = 0             (read only) */  
  byte real;               /* Real analog = 1, integer = 0     (read only) */

  long user;               /* OEM data storage                 (read/write) */

} str_iovar;

/* The data field points to a byte(BOO), a long(ANA) or an array of char(MSG)
   the value buffer can be accessed for writing(input) or reading(output) */

/********* run time board object ***********************/

typedef struct str_iobrd
{
  str_dfbrd *dfbrd;   /* Pointer to board definition             (read only) */
  str_iovar *iovar;   /* Pointer to first variable of this board (read only) */
  void *oem_data;     /* Pointer to oem parameters               (read only) */ 
  void (*oem_call)(); /* Board management procedure              (read/write) */

  word key;           /* OEM key code                            (read only) */

  byte nbvar;         /* Number of connected variables           (read only) */   
  byte input;         /* Input scanning required                 (read/write) */
  byte output;        /* Output refresh required                 (read/write) */ 
  byte lock;          /* Variable(s) locked = 1, else = 0        (read only) */

  long user;          /* OEM data storage                        (read/write) */
  long user2;         /* OEM data storage                        (read/write) */
} str_iobrd;

/* The OEM data field points after the board definition, where oem additional 
   parameters are readable */

/********* run time data base format ***********************/

typedef struct
{
  word nb_brd;             /* Total number of boards */
  word nb_boo;             /* Total number of boo io variables */      
  word nb_ana;             /* Total number of ana io variables */
  word nb_msg;             /* Total number of msg io variables */

  str_dfbrd *first_dfbrd;  /* Pointer to first board definiton */ 
  str_dfvar *first_dfboo;  /* Pointer to first boo variable definiton */ 
  str_dfvar *first_dfana;  /* Pointer to first ana variable definiton */ 
  str_dfvar *first_dfmsg;  /* Pointer to first msg variable definiton */ 

  str_iobrd *first_iobrd;  /* Pointer to first board object */ 
  str_iovar *first_ioboo;  /* Pointer to first boo variable object */ 
  str_iovar *first_ioana;  /* Pointer to first ana variable object */ 
  str_iovar *first_iomsg;  /* Pointer to first msg variable object */ 

} str_ios;

#pragma pack()

/* eof */

