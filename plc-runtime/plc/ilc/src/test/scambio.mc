
(* Versione 2.1 *)

VAR_GLOBAL
   RESET_PLC                 AT %MX5     : BOOL ;
   CLEAR_PLC                 AT %MX6     : BOOL ;
   RIPCIC                    AT %MX40    : ARRAY[0..5] OF BOOL ;
   SOSPCIC                   AT %MX72    : ARRAY[0..5] OF BOOL ;
   TASTI                     AT %MX3069  : ARRAY[0..31] OF BOOL ;
   OVR                       AT %MD3116  : ARRAY[0..3] OF DINT ;
END_VAR

(* Note vers 2.1: *)
(* aggiunto PN_ANT in scambdin che era rimasto non definito *)
