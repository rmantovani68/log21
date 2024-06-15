/*
  Name : grcn0def.h
  Conversion functions standard include file
*/

#pragma pack(1)

#define TRUE   1
#define FALSE  0
#define CR_OK  0
#define CR_BAD -1

#define DIR_INPUT 0                    /* direction = input conversion  */
#define DIR_OUTPUT 1                   /* direction = output conversion */

typedef long   T_ANA;                  /* integer ANA type              */
typedef float  T_REAL;                 /* floating ANA type             */

typedef struct {                       /* conversion structure          */
      unsigned short number;           /* conversion number (reserved)  */
      unsigned short direction;        /* conversion direction          */
      T_REAL *before;                  /* value before conversion       */
      T_REAL *after;                   /* value after conversion        */
   } str_cnv;

#define ARG_BEFORE (*(arg->before))
#define ARG_AFTER  (*(arg->after))
#define DIRECTION  (arg->direction)

#pragma pack()

/* eof */
