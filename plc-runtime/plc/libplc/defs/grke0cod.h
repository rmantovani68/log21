 








#define IL_BEGIN 0xFF00 
#define IL_PUSH 0xFF02 

#define B_INT_POP 1 
#define B_OUT_POP 2 
#define B_OUT_PSH 4 
#define B_INT_IOR 8 
#define B_INT_AND 9 
#define B_INT_XOR 10 
#define B_STA_NOT 12 
#define B_INT_EQU 11 
#define B_OUT_SET 18 
#define B_OUT_RST 19 
#define B_INT_RED 20 
#define B_INT_FED 21 
#define B_INT_SET 22 
#define B_INT_RST 23 
#define B_INP_ACQ 28 
#define B_FLS_PSH 72 
#define B_TRU_PSH 73 

#define A_INT_POP 30 
#define A_OUT_POP 31 
#define A_INP_PSH 33 
#define A_CST_PSH 35 
#define A_INT_IOR 36 
#define A_INT_AND 37 
#define A_INT_XOR 38 
#define A_INT_EQU 39 
#define A_INT_GTH 40 
#define A_INT_LTH 41 
#define A_INT_ADD 42 
#define A_INT_SUB 43 
#define A_INT_MUL 44 
#define A_INT_DIV 45 
#define A_INP_ACQ 52 
#define A_INT_NOS 54 
#define A_SYS_PSH 67 

#define R_INP_ACQ 105 
#define R_INT_SUB 106 
#define R_INT_MUL 107 
#define R_INT_DIV 108 
#define R_INT_ADD 109 
#define R_INT_LTH 110 
#define R_INT_GTH 111 
#define R_INT_EQU 112 
#define R_INT_POP 113 
#define R_OUT_POP 114 
#define R_INP_PSH 116 
#define R_CST_PSH 118 
#define R_STA_NOT 119 

#define T_INT_POP 61 
#define T_INT_PSH 64 
#define T_CST_PSH 66 
#define T_INT_GTH 68 
#define T_INT_LTH 69 
#define T_INT_ADD 70 
#define T_INT_SUB 71 

#define M_INT_POP 77 
#define M_OUT_POP 78 
#define M_INP_PSH 80 
#define M_CST_PSH 82 
#define M_INT_EQU 83 
#define M_INT_LTH 84 
#define M_INT_GTH 85 
#define M_INP_ACQ 86 

#define FB_ACTIVE 120 
#define FB_READPA 121 

#define X_INT_PSH 6 
#define X_INT_TMR 7 
#define B_IXO_OEM 13 
#define A_IXO_OEM 14 
#define M_IXO_OEM 15 

#define _IDTPRI 88 
#define _BACREC 89 
#define _RETURN 91 
#define _BACACT 92 
#define _CONBRA 94 
#define _UCOBRA 95 
#define _POPRET 96 
#define _BREAKP 100  


#define A_B_CONV 1 
#define M_B_CONV 2 
#define B_A_CONV 3 
#define T_A_CONV 4 
#define M_A_CONV 5 
#define B_T_CONV 6 
#define A_T_CONV 7 
#define M_T_CONV 8 
#define B_M_CONV 9 
#define A_M_CONV 10 
#define T_M_CONV 11 

#define TMR_STRT 12 
#define TMR_STOP 13 
#define GRA_STRT 14 
#define GRA_LFRE 15 
#define GRA_LRST 16 
#define GRA_LKIL 17 
#define USP_STRT 21 
#define MSG_CCAT 30 
#define BOO_REDG 31 
#define BOO_FEDG 32 
#define DIO_VALI 35 
#define GRA_STAT 43 
#define PRG_CALF 45 
#define USF_STRT 49 
#define R_B_CONV 50 
#define R_A_CONV 51 
#define R_T_CONV 52 
#define B_R_CONV 54 
#define A_R_CONV 55 
#define T_R_CONV 56 

