
(* Versione 2.4 *)

VAR_GLOBAL

   EMERGENZA                 AT %MX*     : BOOL ;
   ASSICAL                   AT %MX*     : BOOL ;
   RESPLC                    AT %MX*     : BOOL ;
   CNOK                      AT %MX*     : BOOL ;
   CPE                       AT %MX*     : BOOL ;
   STOP                      AT %MX*     : BOOL ;
   AZZERAMENTO               AT %MX*     : BOOL ;
   MDI                       AT %MX*     : BOOL ;
   AUTOMATICO                AT %MX*     : BOOL ;
   MANUALE                   AT %MX*     : BOOL ;
   LPL                       AT %MD*     : DINT ;
   EXIT_VAL                  AT %MD*     : DINT ;
   PAESE                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   RICFIN                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   MODO                      AT %MX*     : ARRAY[0..15] OF BOOL ;
   ABINV                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   COMINV                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   TS                        AT %MX*     : ARRAY[0..15,0..31] OF BOOL ;
   TP                        AT %MX*     : ARRAY[0..15,0..31] OF BOOL ;
   TH                        AT %MX*     : ARRAY[0..15,0..31] OF BOOL ;
   T                         AT %MX*     : ARRAY[0..15,0..79] OF BOOL ;
   INVERR                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   INVES                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   PRNEW                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   DATVAL                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   PIC                       AT %MX*     : ARRAY[0..15] OF BOOL ;
   AXPOS                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   S                         AT %MD*     : ARRAY[0..15] OF REAL ;
   M                         AT %MD*     : ARRAY[0..15] OF DINT ;
   KA                        AT %MD*     : ARRAY[0..15] OF DINT ;
   ORIG_ATT                  AT %MD*     : ARRAY[0..15] OF DINT ;
   ORIG                      AT %MD*     : ARRAY[0..15] OF DINT ;
   INV                       AT %MD*     : ARRAY[0..15,0..6] OF DINT ;
   LZ                        AT %MD*     : ARRAY[0..15] OF DINT ;
   LY                        AT %MD*     : ARRAY[0..15] OF DINT ;
   LX                        AT %MD*     : ARRAY[0..15] OF DINT ;
   OUT_RANGE                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_WPLC               AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_TS                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_TP                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_TH                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_T                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_S                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_M                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   STOPINV                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   QUOX_FUT                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   KB3                       AT %MX*     : ARRAY[0..15] OF BOOL ;
   KB2                       AT %MX*     : ARRAY[0..15] OF BOOL ;
   KB_KO                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   KB_OK                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   STROBE_COM                AT %MX*     : ARRAY[0..15] OF BOOL ;
   COMMAND                   AT %MD*     : ARRAY[0..15] OF DINT ;
   SELVEL                    AT %MX*     : ARRAY[0..99] OF BOOL ;
   MENO                      AT %MX*     : ARRAY[0..99] OF BOOL ;
   PIU                       AT %MX*     : ARRAY[0..99] OF BOOL ;
   HOLDAX                    AT %MX*     : ARRAY[0..99] OF BOOL ;
   ABIAX                     AT %MX*     : ARRAY[0..99] OF BOOL ;
   ASSCOM                    AT %MX*     : ARRAY[0..99] OF BOOL ;
   AZZOK                     AT %MX*     : ARRAY[0..99] OF BOOL ;
   ASSMOT                    AT %MX*     : ARRAY[0..99] OF BOOL ;
   OVERR                     AT %MD*     : ARRAY[0..99] OF DINT ;
   AZZ                       AT %MX*     : ARRAY[0..99] OF BOOL ;
   CAMMA                     AT %MX*     : ARRAY[0..99,0..4] OF BOOL ;
   GENERAL                   AT %MD*     : ARRAY[0..63] OF DINT ;
   QUOTA                     AT %MD*     : ARRAY[0..15] OF REAL ;
   RIC_DAT_ANT               AT %MX*     : BOOL ;
   DAT_ERR                   AT %MX*     : BOOL ;
   DATVAL_ANT                AT %MX*     : BOOL ;
   KA_ANT                    AT %MD*     : DINT ;
   LZ_ANT                    AT %MD*     : DINT ;
   LY_ANT                    AT %MD*     : DINT ;
   LX_ANT                    AT %MD*     : DINT ;
   ACK_GRU                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   STOP_GRU                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   START_GRU                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   DEFGR15                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR14                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR13                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR12                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR11                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR10                   AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR9                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR8                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR7                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR6                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR5                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR4                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR3                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR2                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR1                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   DEFGR0                    AT %MD*     : ARRAY[0..7,0..1] OF DINT ;
   FLAGS                     AT %MD*     : ARRAY[0..15] OF DINT ;
   ANT                       AT %MD*     : ARRAY[0..15] OF REAL ;
   ACCEL                     AT %MD*     : ARRAY[0..15] OF REAL ;
   VEL                       AT %MD*     : ARRAY[0..15] OF REAL ;
   LOOPLC                    AT %MX*     : BOOL ;
   ACK_CAR                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   STOP_CAR                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   START_CAR                 AT %MX*     : ARRAY[0..15] OF BOOL ;
   SIMULAZIONE               AT %MX*     : BOOL ;
   AZZMAN                    AT %MX*     : BOOL ;
   AZZCEN                    AT %MX*     : BOOL ;
   AZZSIN                    AT %MX*     : BOOL ;
   CLEAR                     AT %MX*     : BOOL ;
   TIPINT                    AT %MX*     : BOOL ;
   RIC_OP                    AT %MX*     : BOOL ;
   ERR_OP                    AT %MX*     : BOOL ;
   ACK_OP                    AT %MX*     : BOOL ;
   BITINT                    AT %MX*     : BOOL ;
   COD_OP                    AT %MD*     : DINT ;
   ACK_CMD		     AT %MX*     : BOOL ;
   ERR_CMD		     AT %MX*     : BOOL ;
   RIC_CMD		     AT %MX*     : BOOL ;
   EXE			     AT %MX*     : BOOL ;
   COD_CMD		     AT %MD*     : DINT ;
   DATA_CMD		     AT %MD*     : ARRAY[0..3] OF DINT ;
   DATA_OP		     AT %MD*     : ARRAY[0..3] OF DINT ;
   OVERRIDE                  AT %MD*     : DINT ;
   EXEPLC		     AT %MX*     : BOOL ;
   I_MIN                     AT %MX*     : BOOL ;
   I_MAX                     AT %MX*     : BOOL ;
   I_VUOTO                   AT %MX*     : BOOL ;
   I_BLOCCO                  AT %MX*     : BOOL ;
   CONTAM                    AT %MX*     : BOOL ;
   DEC_US                    AT %MX*     : BOOL ;
   INC_US                    AT %MX*     : BOOL ;

   START_ID                  AT %MD*     : ARRAY[0..15] OF UDINT ;
   INHOLD                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   NOHOLD                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   STOPLOOP                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   INT_BUSY                  AT %MX*     : ARRAY[0..15] OF BOOL ;
   EME_CEN                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   XCAMMA1                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   XCAMMA2                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   XCAMMA                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   TGL_PRG                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   RAGGIO                    AT %MD*     : ARRAY[0..15] OF REAL ;
   MODOINT                   AT %MD*     : ARRAY[0..15] OF UDINT ;
   MODOCRU                   AT %MD*     : ARRAY[0..15] OF UDINT ;
   CONSPLC                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   ABINT                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   SEMIAUT                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   BREAK                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   RETRACE                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   IDPRG                     AT %MD*     : ARRAY[0..15] OF UDINT ;
   NUMPAS                    AT %MD*     : ARRAY[0..15] OF UDINT ;
   ATTR_ST                   AT %MD*     : ARRAY[0..15] OF UDINT ;
   STOPLES                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   HLD_AXPR                  AT %MX*     : ARRAY[0..99] OF BOOL ;
   QUOTAS                    AT %MD*     : ARRAY[0..99] OF REAL ;
   TARGET                    AT %MD*     : ARRAY[0..99] OF REAL ;
   STOPAX		     AT %MX*     : ARRAY[0..99] OF BOOL ;
   VOLAX		     AT %MX*     : ARRAY[0..99] OF BOOL ;
   YAS_STS                   AT %MD*     : ARRAY[0..99] OF UDINT ;
   YAS_I_O                   AT %MD*     : ARRAY[0..99] OF UDINT ;
   YAS_ALM                   AT %MD*     : ARRAY[0..99] OF UDINT ;
   SERVOON                   AT %MX*     : ARRAY[0..99] OF BOOL ;
   DRIVEOK                   AT %MX*     : ARRAY[0..99] OF BOOL ;
   TERMPAS                   AT %MX*     : ARRAY[0..99] OF BOOL ;
   GENERIC                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   MSG_CLEAR		     AT %MX*	 : BOOL ;
   MVOL0		     AT %MX*	 : BOOL ;
   MVOL1		     AT %MX*	 : BOOL ;
   SWAP		     	     AT %MX*	 : BOOL ;
   SEMPOST	     	     AT %MX*	 : BOOL ;
   ASSE_SEL                  AT %MD*     : UDINT ;
   CNTPZ                     AT %MD*     : UDINT ;
   PRESET                    AT %MD*     : UDINT ;
   STPLC                     AT %MD*     : UDINT ;
   DISTINTA                  AT %MX*     : BOOL ;
   MESSAGGI                  AT %MX*     : BOOL ;
   INS_RIGA                  AT %MX*     : BOOL ;
   ABLPESO	     	     AT %MX*	 : BOOL ;
   PLCR		     	     AT %MX*	 : BOOL ;
   FLC		     	     AT %MX*	 : BOOL ;
   FLR		     	     AT %MX*	 : BOOL ;
   SCALOVE                   AT %MD*     : UDINT ;
   MODVOL	     	     AT %MX*	 : BOOL ;
   MODVOL1	     	     AT %MX*	 : BOOL ;
   RANK                      AT %MD*     : UDINT ;
   ST_RANK	     	     AT %MX*	 : BOOL ;
   ACK_RANK	     	     AT %MX*	 : BOOL ;
   AZZIC	     	     AT %MX*	 : BOOL ;
   CORREN                    AT %MD*     : UDINT ;
   C_VUOTO                   AT %MD*     : UDINT ;
   C_MIN                     AT %MD*     : UDINT ;
   C_MAX                     AT %MD*     : UDINT ;
   C_MIN_A                   AT %MD*     : UDINT ;
   C_MAX_A                   AT %MD*     : UDINT ;
   C_MIN_V                   AT %MD*     : UDINT ;
   C_MAX_V                   AT %MD*     : UDINT ;
   T_MENO	     	     AT %MX*	 : BOOL ;
   T_PIU	     	     AT %MX*	 : BOOL ;
   T_VEL	     	     AT %MX*	 : BOOL ;
   NOMOV                     AT %MX*     : ARRAY[0..15] OF BOOL ;
   AUTOAPP	     	     AT %MX*	 : BOOL ;
   NCORREN                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_VUOT                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MIN                    AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MAX                    AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MINA                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MAXA                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MINV                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NC_MAXV                   AT %MD*     : ARRAY[0..7] OF UDINT ;
   NI_VUOT                   AT %MX*     : ARRAY[0..7] OF BOOL ;
   NI_MIN                    AT %MX*     : ARRAY[0..7] OF BOOL ;
   NI_MAX                    AT %MX*     : ARRAY[0..7] OF BOOL ;
   NI_BLOC                   AT %MX*     : ARRAY[0..7] OF BOOL ;
   NINC_US                   AT %MX*     : ARRAY[0..7] OF BOOL ;
   NDEC_US                   AT %MX*     : ARRAY[0..7] OF BOOL ;
   NCONTAM                   AT %MX*     : ARRAY[0..7] OF BOOL ;
   ERRORI                    AT %MD*     : UDINT ;
   MODVIS                    AT %MD*     : UDINT ;
   IDPRGANT                  AT %MD*     : UDINT ;
   PN_ANT                    AT %MD*     : BOOL ;
   NCOMINV                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   NABINV                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   NINVES                    AT %MX*     : ARRAY[0..15] OF BOOL ;
   NINVERR                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   NOUT_RA                   AT %MX*     : ARRAY[0..15] OF BOOL ;
   CODINV                    AT %MD*     : ARRAY[0..15] OF UDINT;
   RACINV                    AT %MD*     : ARRAY[0..15] OF REAL ;
   DECINV                    AT %MD*     : ARRAY[0..15] OF REAL ;
   VELINV                    AT %MD*     : ARRAY[0..15] OF REAL ;
   CORREZ                    AT %MD*     : ARRAY[0..99] OF REAL ;
   ABIMESS	     	     AT %MX*	 : BOOL ;
   NRIGA                     AT %MD*     : UDINT ;
   BS_INVCORR                AT %MD*     : ARRAY[0..9] OF REAL ; 
   BS_INVFREQ                AT %MD*     : ARRAY[0..9] OF REAL ;

END_VAR

(* Note vers 2.2: *)
(* Aggiunti NCOMINV NABINV NINVES NINVERR NOUT_RA CODINV RACINV DECINV VELINV *)

(* Note vers 2.3: *)
(* Aggiunti ABIMESS CORREZ *)

(* Note vers 2.4: *)
(* Aggiunti BS_INVCORR BS_INVFREQ *)
