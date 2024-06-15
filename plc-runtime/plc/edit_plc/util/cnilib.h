FUNCTION_BLOCK  OSCIAX

  VAR
	AMPOS : REAL:= 0.0;  (* Ampiezza oscillazione *)
	DELTAOS : REAL:= 0.0;  (* Step di spostamento oscillazione *)
	ENDPOS : BOOL:= 0;  (* Fronte down ACK_GRU = Asse OS pos. *)
	ENDTIME : BOOL:= 0;  (* Fine tempo di attesa *)
	FL_FREC : BOOL:= 0;  (* Freccia ON di Inizio/Fine *)
	FRECC_A : REAL:= 0.0;  (* Freccia di Apertura *)
	FRECC_C : REAL:= 0.0;  (* Freccia di Chiusura *)
	GOPOS : BOOL:= 0;  (* Fronte up ACK_GRU = Asse OS com. *)
	GOTIME : BOOL:= 0;  (* Start timer attesa SX,CR,DX *)
	PRIMOOS : BOOL:= 0;  (* Primo posizionamento *)
	QTAOS : REAL:= 0.0;  (* Quota obiettivo *)
	RUN_OSC : BOOL:= 0;  (* Flags di oscillatore in start *)
	TMPCR : DINT:= 0;  (* Tempo attesa al Centro *)
	TMPDX : DINT:= 0;  (* Tempo di attesa Destra *)
	TMPSX : DINT:= 0;  (* Tempo attesa SX *)
	ULTIMOS : BOOL:= 0;  (* Ultimo posizionamento *)
	VALTIME : DINT:= 0;  (* Valore di attesa (millisecondi) *)
	VELOS : REAL:= 0.0;  (* Velocita' Oscillazione *)
	FS_ACKG : R_TRIG;
	ATTESA : TON;
	FD_ACKG : F_TRIG;
  END_VAR

  VAR_INPUT
	AMPO : REAL:= 0.0;  (* Ampiezza Oscillazione *)
	AXOS : DINT:= 0;  (* Numero d'Ordine Asse Oscillatore *)
	FRCA : REAL:= 0.0;  (* Freccia di apertura *)
	FRCC : REAL:= 0.0;  (* Freccia di chiusura *)
	STARTO : BOOL:= 0;  (* Abilitazione START oscillatore *)
	STOPO : BOOL:= 0;  (* Abilitazione STOP oscillatore *)
	TACR : DINT:= 0;  (* Tempo attesa Centro *)
	TADX : DINT:= 0;  (* Tempo attesa Sx *)
	TASX : DINT:= 0;  (* Tempo attesa Sx *)
	VELO : REAL:= 0.0;  (* Velocita' oscillazione *)
  END_VAR


(* --------------------------------------------------------- *)
(*      Versione 2.1 :gestita oscillazione nulla e sul reset *)
(*		      aggiunto l'attivazione di STOP_GRU     *)
(*      Function block di gestione                           *)
(*      movimentazione asse oscillante                       *)
(*	Se STARTO=STOPO=1 --> modifica dati oscillatore      *)
(*      N.B. Usa il raggruppamento numero 7                  *)
(*                                                           *)
(*	INPUT:  STARTO=abilitazione start oscillatore        *)
(*		STOPO=abilitazione stop oscillatore          *)
(*		AXOS=numero asse che deve oscillare          *)
(*		AMPO=ampiezza oscillazione                   *)
(*		VELO=velocita' oscillazione                  *)
(*		TASX=tempo attesa sx                         *)
(*		TACR=tempo attesa centro                     *)
(*		TADX=tempo attesa sx                         *)
(*		FRCA=freccia di apertura                     *)
(*		FRCC=freccia di chiusura                     *)
(*	OUTPUT:	Nessuno	                                     *)
(* --------------------------------------------------------- *)


(* Gestione Timer di attesa Sinistra Centro Destra *)
(* ----------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK APPON

  VAR
	ACONDIZ : BOOL:= 0;  (* Flags di OK per entrata autoap. *)
	CE_MATM : DINT:= 0;  (* Codice Errore sel. modo automatico *)
	CE_SIAP : DINT:= 0;  (* Codice Errore entrata autoap. *)
	CMDMATM : BOOL:= 0;  (* Flags comando sel. modo automatico *)
	CMDSIAP : BOOL:= 0;  (* Flags comando entrata autoap. *)
	ER_MATM : BOOL:= 0;  (* Errore su sel. modo automatico *)
	ER_SIAP : BOOL:= 0;  (* Errore su entrata autoapprendimento *)
	OK_MATM : BOOL:= 0;  (* Fine comando sel. modo automatico *)
	OK_SIAP : BOOL:= 0;  (* Fine comando entrata autoap. *)
	RUNCMD : BOOL:= 0;  (* Flags di comando in corso *)
	CSIAP : CMDGEST;
	CMATM : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* Abilitazione entrata autoap. (edit) *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando entrata autoap. *)
	ERRCMD : BOOL:= 0;  (* Errore comando entrata autoap. *)
  END_VAR
	
(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      entrata in autoapprendimento della macchina          *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione entrata autoap.          *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  _CARDAM

  VAR
	FLING : BOOL:= 0;  (* Flag almeno un ingresso *)
	FLUSC : BOOL:= 0;  (* Flag almeno un uscita *)
	INDZ : DINT:= 0;  (* Indice in lista zona *)
	MAX : DINT:= 0;  (* Numero massimo di zona *)
	MIN : DINT:= 0;  (* Numero minimo di zona *)
	NC : DINT:= 0;  (* indice centro *)
	NC1 : DINT:= 0;
	RESET : _RESDIS;
  END_VAR

  VAR_OUTPUT
	CODERR : DINT:= 0;  (* Codice errore *)
	ERRORE : BOOL:= 0;  (* Flag errore *)
  END_VAR

	(*						*)
	(*						*)
	(*	CARICAMENTO MATRICE _ZONA		*)
	(*						*)
	
	(* OUTPUT:	ERRORE		flag errore	*)
	(*		se ERRORE=1			*)
	(*		CODERR=1	numero centri <2	*)
	(*		CODERR=2	nessun ingresso		*)
	(*		CODERR=3	nessuna uscita		*)
	(*		CODERR=4	deve essere context=2	*)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK  GESTDIS

  VAR
	CTP : DINT:= 0;  (* Contapezzi *)
	ENAB : BOOL:= 0;  (* abilitazione datcou *)
	FCPE : BOOL:= 0;  (* Fronte salita CPE *)
	FDIS : BOOL:= 0;  (* Fronte discesa PIC o INT_BUSY *)
	FSPI : BOOL:= 0;  (* Fronte salita PIC *)
	ID : DINT:= 0;  (* Identificatore linea *)
	NCEN : DINT:= 0;  (* Indice centro *)
	ZONE : DINT:= 0;  (* Zona del centro interessato *)
	GETLINE : _GETLIN;
	FRCPE : R_TRIG;
	RESVA1 : _RESDIS;
	RESVAR : _RESDIS;
	FSPIC : R_TRIG;
	INN : _INNESCO;
	FDINT : F_TRIG;
	INNES : _REINNE;
	DATEX : CMDGEST;
	DATCOU : CMDGEST;
	LEGGI : _CARDAM;
	GCOMAN : _GETCMD;
	FDPIC : F_TRIG;
  END_VAR

  VAR_INPUT
	MODE : BOOL:= 0;  (* Modalita' (forzatura evoluzione) *)
	NC : DINT:= 0;  (* Numero centro *)
	ORG : DINT:= 0;  (* Nr. origine da mettere in start *)
	START : BOOL:= 0;  (* Bit di start *)
	XD : DINT:= 0;  (* Dritta/speculare in X *)
	XN : BOOL:= 0;  (* Normale/complementare in X *)
	YD : DINT:= 0;  (* Dritta/speculare in Y *)
	YN : DINT:= 0;  (* Normale/complementare in Y *)
	ZD : DINT:= 0;  (* Dritta/speculare in Z *)
	ZN : DINT:= 0;  (* Normale/complementare in Z *)
  END_VAR

  VAR_OUTPUT
	CODERR : DINT:= 0;  (* Codice errore *)
	ERRORE : BOOL:= 0;  (* Flag errore *)
  END_VAR

	(* Gestione distinta *)
	
	(* INPUT:	NC		Nr. centro								*)
	(*		START		start:	0	no comando						*)
	(*		MODE		modo:	0	no forzatura evoluzione distinta			*)
	(*					1	evoluzione distinta					*)
	(*		org		Nr. origine	0 ==> no origine					*)
	(*						1,2,..							*)
	(*													*)
	(*		XD		Attributo X		0 ==> origine dritta 	1 origine speculare	*)
	(*		XN		Attributo X		0 ==> origine normale 	1 origine complementare *)			
	(*		YD		Attributo Y		0 ==> origine dritta 	1 origine speculare	*)					
	(*		YN		Attributo Y		0 ==> origine normale 	1 origine complementare *)
	(*		ZD		Attributo Z		0 ==> origine dritta 	1 origine speculare	*)			
	(*		ZN		Attributo Z		0 ==> origine normale 	1 origine complementare *)					

	(* OUTPUT:	ERRORE		flag errore (=1 se errore) 						*)
	(*		CODERR		Codice errore 								*)
	(*				1		operazione non abilitata 				*)
	(*				2		operazione non possibile 				*)
	(*				3		identificatore riga non trovato o non valido	 	*)
	(*				4		comando non gestito 					*)
	(*				11		operazione non abilitata 				*)
	(*				12		identificatore non trovato 				*)
	(*				13		macchina non in start 					*)
	(*				15		centro gia' in start 					*)
	(*				21		Inaspettata ricezione identificatore riga 		*)
	
	(*				30		Nr. di centro in input errato 				*)
	
	(*				41		Nr. centri < 2 						*)
	(*				42		Nessun centro di ingresso				*)
	(*				43		Nessun centro di uscita					*)
	(*				44		Deve essere context=2					*)
	

END_FUNCTION_BLOCK

FUNCTION_BLOCK RISPCMD

  VAR_INPUT
	CODE : DINT:= 0;  (* Codice in risposta *)
	DATO0 : DINT:= 0;  (* Dato 0 in risposta *)
	DATO1 : DINT:= 0;  (* Dato 1 in risposta *)
	DATO2 : DINT:= 0;  (* Dato 2 in risposta *)
	DATO3 : DINT:= 0;  (* Dato 3 in risposta *)
  END_VAR

	(*						*)
	(*						*)
	(*	Gestione risposta a dati da CN		*)
	(*						*)
	(*	INPUT:	CODE	Codice in risposta	*)
	(*		DATO0	Dato 0 in risposta	*)
	(*		DATO1	Dato 1 in risposta	*)
	(*		DATO2	Dato 2 in risposta	*)
	(*		DATO3	Dato 3 in risposta	*)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK GEST_M

  VAR
	DM : BOOL:= 0;  (* Appoggio decodifica M *)
	NCEN : DINT:= 0;  (* Indice centro *)
	NUMCEN : DINT:= 0;  (* numero centri *)
	SBL : BOOL:= 0;  (* Appoggio sblocco M *)
	_FSBLO : R_TRIG;
	_FDECM : R_TRIG;
  END_VAR

  VAR_INPUT
	CODE : DINT:= 0;  (* Codice comando *)
	COND : BOOL:= 0;  (* Condizione di blocco *)
	EMME : DINT:= 0;  (* nr. M da decodificare *)
	NC : DINT:= 0;  (* Nr. centro *)
  END_VAR

  VAR_OUTPUT
	DECM : BOOL:= 0;  (* Notifica decodifica M richiesta *)
	SBLO : BOOL:= 0;  (* Fronte salita sblocco M richiesta *)
  END_VAR

(* Gestione rilevazione istruzione M *)
(* Input	: CODE = Codice comando *)
(*		  CODE = 1	Predisposizione *)
(*		  CODE = 2	Decodifica M *)

(*		  ulteriori input con CODE=2 *)

(*		  NC   = Numero centro (0,1,2 ...) *)
(*		  EMME = Numero M da decodificare *)
(*		  COND = Stato condizione (0=blocco, 1=sblocco) *)

(*		  output con CODE=2 *)

(*		  DECM = fronte salita su decodifica M richiesta *)
(*		  SBLO = fronte salita su sblocco M richiesta *)

(*		  CODE = 3	Generazione segnale PAESE *)

(* Il main dovra' avere la seguente struttura : *)

(* LD	1 *)
(* ST	GM.CODE *)
(* CAL	GM *)

(* .... *)
(* .... *)

(* Gestione M35 centro 0 *)

(* LD	2 *)
(* ST	GM.CODE *)
(* LD	0 *)
(* ST	GM.NC *)
(* LD	35 *)
(* ST	GM.EMME *)
(* LD	BLOCCO *)
(* ST	GM.COND *)
(* CAL	GM *)

(* Gestione M40 centro 1*)

(* LD	2 *)
(* ST	GM.CODE *)
(* LD	1 *)
(* ST	GM.NC *)
(* LD	40 *)
(* ST	GM.EMME *)
(* LD	BLOCCO1 *)
(* ST	GM.COND *)
(* CAL	GM *)

(* .... *)
(* .... *)


(* LD	3 *)
(* ST	GM.CODE *)
(* CAL	GM *)




(* Decodifica Comando *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  DIGIT

  VAR
	ABGRU : BOOL:= 0;  (* Abilitazione gruppo *)
	ABIREC : BOOL:= 0;  (* Abilitazione recupero *)
	AXDIG : DINT:= 0;  (* Nr. interno asse trasporto *)
	CHANGE : BOOL:= 0;  (* Flag chenge dati dinamici *)
	CONTR : BOOL:= 0;  (* Flag controllo *)
	CORRENT : LREAL:= 0.0;  (* Correzione in entrata (mm) *)
	CORRET : LREAL:= 0.0;  (* Correttore *)
	CORRUS : LREAL:= 0.0;  (* Correzione in uscita (mm) *)
	DELTA : LREAL:= 0.0;  (* Diff ingresso/uscita micro FC *)
	DELTAQ : LREAL:= 0.0;  (* Variazione quota trasporto *)
	DISCESE : ARRAY[0..31] OF LREAL:= [32(0.0)];  (* Coda fronti discesa fine corsa *)
	DISTFC : LREAL:= 0.0;  (* Distanza FC - gruppo di lavoro *)
	ENRECU : BOOL:= 0;  (* Enable recupero *)
	ENTRATA : LREAL:= 0.0;  (* Quota entrata *)
	INDPREL : DINT:= 0;  (* Indice prelievo in coda *)
	INDSCA1 : DINT:= 0;  (* Indice scaricamento coda discese *)
	INDSCAR : DINT:= 0;  (* Indice scaricamento in coda salite *)
	NOCORR : LREAL:= 0.0;  (* Quota target non corretta *)
	OBIETT : LREAL:= 0.0;  (* Obiettivo *)
	OSCILL : BOOL:= 0;
	PRES : LREAL:= 0.0;  (* Preset metri lavorati *)
	PRIMOL : BOOL:= 0;  (* Primo loop *)
	PZSOTTO : BOOL:= 0;
	QUOLD : LREAL:= 0.0;  (* Valore old quota *)
	QUOT : LREAL:= 0.0;
	ROLLO : LREAL:= 0.0;  (* Modulo roll-over *)
	SALITE : ARRAY[0..31] OF LREAL:= [32(0.0)];  (* Coda fronti salita fine corsa *)
	SROT : BOOL:= 0;  (* Senso rotazione (0=dx 1=sx) *)
	T200MS : BOOL:= 0;
	TENT : DINT:= 0;  (* Tipo entrata *)
	TREC : TIME:= T#0s;  (* Preset timer recupero *)
	TUSC : DINT:= 0;  (* Tipo uscita *)
	USCITA : LREAL:= 0.0;  (* Quota uscita *)
	VELENT : LREAL:= 0.0;  (* Velocita' correzione in entrata *)
	VELUS : LREAL:= 0.0;  (* Velocita' correzione in uscita *)
	VRMAX : LREAL:= 0.0;  (* Velocita' rotazione massima *)
	VROT : LREAL:= 0.0;  (* Velocita' rotazione *)
	TPZSOT : TOF;
	TRECU : TON;
  END_VAR

  VAR_INPUT
	FC : BOOL:= 0;  (* Micro fine corsa *)
	FD : BOOL:= 0;  (* Fronte discesa FC filtrato *)
	FS : BOOL:= 0;  (* fronte salita FC filtrato *)
	GRUPPO : DINT:= 0;  (* Nr. gruppo di lavorazione (1,..) *)
	VELOC : LREAL:= 0.0;  (* Velocita' trasporto filtrata *)
  END_VAR

  VAR_OUTPUT
	INTERV : BOOL:= 0;  (* Out intervento *)
	MANDR : BOOL:= 0;  (* Comando rotazione mandrino *)
	OSC : BOOL:= 0;  (* Abilitazione oscillazione nastro *)
	RECUP : BOOL:= 0;  (* Out recupero *)
	SENROT : BOOL:= 0;  (* Senso rotazione (0=dx 1=sx) *)
	VELROT : DINT:= 0;  (* Velocita' rotazione (16 bits) *)
  END_VAR

(* ---------------------------------------------------------------------- *)
(* Versione 2.0                                                           *)
(* Function block per gestione digitali                                   *)
(* 									  *)
(* Questa function block si preoccupa di attivare e disattivare		  *)
(* l'intervento (INTERV) in base ai fronti di salita e discesa		  *)
(* del fine corsa.							  *)
(* Gestisce anche le informazioni di out rotazione mandrino,		  *)
(* senso di rotazione e velocita' rotazione				  *)
(*                                                                        *)
(* INPUT:	GRUPPO		nr. gruppo di lavoro + 1 (1,2,..)         *)
(* 		FC		input fc				  *)
(*		FS		fronte salita input fc filtrato           *)
(*		FD		fronte discesa input fc filtrato          *)
(*		VELOC		velocita' trasporto filtrata              *)
(* OUTPUT:	MANDR		out mandrino                              *)
(*		INTERV		out intervento                            *)
(*		RECUP		out recupero                              *)
(*		SENROT		senso rotazione                           *)
(*		VELROT		(16 bits)                                 *)
(*		OSC		abilitazione oscillazione nastro	  *)
(* ---------------------------------------------------------------------- *)

(* Test primo loop *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  _GETLIN

  VAR
	ENAB : BOOL:= 0;  (* innesco richiesta identificatore linea *)
	LP : DINT:= 0;  (* indice prelievo in coda linee *)
	NCEN : DINT:= 0;  (* Nr. centro *)
	GETL : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* innesco gestlin *)
	MODE : DINT:= 0;  (* Modalita' *)
	NC : DINT:= 0;  (* Nr. centro *)
  END_VAR

  VAR_OUTPUT
	CODERR : DINT:= 0;  (* Codice errore in uscita *)
	CTP : DINT:= 0;  (* Contapezzi in out *)
	ESITO : DINT:= 0;  (* Esito in uscita *)
	ID : DINT:= 0;  (* Id in out *)
	QTA : DINT:= 0;  (* Preset *)
  END_VAR

	(*							*)
	(*							*)
	(*	Get identificatore linea distinta		*)
	(*							*)
	(*	INPUT:	ENABLE	innesco richiesta		*)
	(*		NC	numero centro			*)
	(*		MODE	=0	contronta ctpz e qta	*)
	(*			=1	richiede linea succ.	*)
	(*	OUTPUT:	ESITO	0	attendere		*)
	(*			1	okay			*)
	(*			2	errore			*)
	
	(*	se ESITO=1					*)
	(*		ID	identificatore linea		*)
	(*		ID=-1	se identificatori esauriti	*)
	(*		CTP	contapezzi linea		*)
	(*		QTA	quantita'			*)
	
	(*	se ESITO=2					*)
	(*		CODERR	codice errore			*)
	

	(* carica defaults *)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK APPOFF

  VAR
	CE_NOAP : DINT:= 0;  (* Codice Errore uscita autoap. *)
	ER_NOAP : BOOL:= 0;  (* Errore su uscita autoap. *)
	OK_NOAP : BOOL:= 0;  (* Fine comando uscita autoap. *)
	CNOAP : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* Abilitazione uscita autoap. (EDIT) *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando uscita autoap. *)
	ERRCMD : BOOL:= 0;  (* Errore comando uscita autoap. *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      uscita dall' autoapprendimento della macchina        *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione uscita autoap.           *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK RXCMD

  VAR_OUTPUT
	CODE : DINT:= 0;  (* Codice ricevuto *)
	DATO0 : DINT:= 0;  (* Dato 0 ricevuto *)
	DATO1 : DINT:= 0;  (* Dato 1 ricevuto *)
	DATO2 : DINT:= 0;  (* Dato 2 ricevuto *)
	DATO3 : DINT:= 0;  (* Dato 3 ricevuto *)
	RX : BOOL:= 0;  (* Flag dati ricevuti *)
  END_VAR

	(*								*)
	(*	Gestione dati da CN					*)
	(*								*)
	(*	OUTPUT:	RX	flag dati ricevuti (1=dati presenti)	*)
	(*		CODE	codice ricevuto				*)
	(*		DATO0	dato 0 ricevuto				*)
	(*		DATO1	dato 1 ricevuto				*)
	(*		DATO2	dato 2 ricevuto				*)
	(*		DATO3	dato 3 ricevuto				*)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK AZZMANU

  VAR
	CE_AZZM : DINT:= 0;  (* Codice Errore azzeramento manuale *)
	ER_AZZM : BOOL:= 0;  (* Errore su azzeramento manuale *)
	OK_AZZM : BOOL:= 0;  (* Fine comando azzeramento manuale *)
	CAZZM : CMDGEST;
  END_VAR

  VAR_INPUT
	ASSEZM : DINT:= 0;  (* Numero asse da azzerare manualmente *)
	ENABLE : BOOL:= 0;  (* Abilitazione azzeramento manuale *)
	ZQUOTA : REAL:= 0.0;  (* Quota di azzeramento manuale *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando azzeramento manaule *)
	ERRCMD : BOOL:= 0;  (* Errore comando azzeramento manaule *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      azzeramento manuale di un asse della macchina        *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione azz. manuale asse        *)
(*		ASSEZM=Asse da azzerare manualmente          *)
(*		ZQUOTA=Quota di azzeramento                  *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  _GETCMD

  VAR
	NC : DINT:= 0;  (* Numero centro *)
	GETL : CMDGEST;
	RCMD : RISPCMD;
	GCMD : RXCMD;
  END_VAR

  VAR_OUTPUT
	CODERR : DINT:= 0;  (* Codice errore *)
	ESITO : BOOL:= 0;  (* Flag errore *)
  END_VAR

	(* Gestione comandi da CN *)
	
	(* OUTPUT:	ESITO		Flag errore (=1 se errore) *)
	(*		CODERR		Codice errore:		1		Operazione non abilitata *)
	(*							2		Identificatore non trovato *)
	(*							3		Macchina non in start *)
	(*							5		Centro gia' in start *)
	(*							11		Ricevuta codice 1 con _RICIN=0 *)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK APPASSO

  VAR
	CE_APSS : DINT:= 0;  (* Codice Errore autoaprendi passo *)
	CMDAPSS : BOOL:= 0;  (* Comando autoaprendi passo inviato *)
	ER_APSS : BOOL:= 0;  (* Errore su autoaprendi passo *)
	OK_APSS : BOOL:= 0;  (* Fine comando autoaprendi passo *)
	CAPSS : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* Abilitazione autoapp. passo *)
	PARATP : BOOL:= 0;  (* Flags di tabella ParAtp *)
	VLBR3D : REAL:= 0;  (* Valore del Raccordo BR3D *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando autoapprendi passo *)
	ERRCMD : BOOL:= 0;  (* Errore comando autoapprendi passo *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      autoapprendimento di un passo di programma           *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione autoap. passo            *)
(*		VLBR3D=Valore del raccordo BR3D (-1=no BR3D) *)
(*		PARATP=Flags si dati tabella PARAPLC         *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  UTILDIG

  VAR
	DELTAME : LREAL:= 0.0;
	DELTAQ : LREAL:= 0.0;
	ENABLE : BOOL:= 0;  (* Abilitazione timer *)
	FCOLD : BOOL:= 0;  (* Copia stato fine corsa *)
	FINECO : BOOL:= 0;  (* Fine ritardo *)
	FS1 : BOOL:= 0;
	OLDQ : LREAL:= 0.0;  (* Valore old quota *)
	PRIMOL : BOOL:= 0;  (* Primo loop *)
	QUOT : LREAL:= 0.0;  (* Quota tasporto *)
	ROLLOX : LREAL:= 0.0;  (* Rollover asse trasporto *)
	RTD : BOOL:= 0;
	VELOX : ARRAY[0..7] OF LREAL:= [8(0.0)];  (* Buffer velocita' *)
	FILTRO : TON;
	RITARDO : TON;
	FSAL1 : R_TRIG;
	CPARZ : DINT:= 0;  (* Contapezzi parziale *)
  END_VAR

  VAR_INPUT
	AX : DINT:= 0;  (* Nr. interno asse trasporto *)
	CING : DINT:= 1;  (* Nr. cingolo (1,2,..) *)
	FC : BOOL:= 0;  (* Micro fine corsa *)
	RESCTP : BOOL:= 0;  (* reset contapezzi parziale *)
	TEMPO : TIME:= T#0ms;  (* preset (ms) filtro fine corsa *)
  END_VAR

  VAR_OUTPUT
	FD : BOOL:= 0;  (* Fronte discesa micro fine corsa *)
	FS : BOOL:= 0;  (* Fronte salita micro fine corsa *)
	VELOC : LREAL:= 0.0;  (* Velocita' trasporto *)
	CTPEZ : DINT:= 0;  (* Contapezzi parziale *)
  END_VAR

(* ---------------------------------------------------------------------- *)
(* Versione 2.1                                                           *)
(* Function block di utilita' per i digitali                              *)
(* questa funzione realizza le seguenti funzioni:                         *)
(* 1) rilevazione della velocita' del trasporto (media sulle ultime       *)
(*    8 velocita' con filtro 100 msecs. e scrittura nel dato macchina)    *)
(* 2) gestione fronti salita e discesa del fine corsa con filtro di tempo *)
(*    programmabile                                                       *)
(* 3) gestione contapezzi parziale e totale e scrittura nel dato macchina *)
(* 4) gestione kilometri lavorati e scrittura nel dato macchina           *)
(*                                                                        *)
(* INPUT:	AX		nr. interno asse trasporto                *)
(*		CING		nr. cingolo (1,2,..)			  *)
(*		FC		input fine corsa                          *)
(*		TEMPO		tempo filtro per input fc                 *)
(*		RESCTP		reset contapezzi parziale		  *)
(* OUTPUT:	FS		fronte salita input fc filtrato           *)
(*		FD		fronte discesa input fc filtrato          *)
(*		VELOC		velocita' trasporto                       *)
(* ---------------------------------------------------------------------- *)

(* Test primo loop *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK AZZGLOB

  VAR
	ACONDIZ : BOOL:= 0;  (* Flags di condizioni OK per azz. *)
	CE_AZGL : DINT:= 0;  (* Codice errore sel. azz. globale *)
	CE_MODA : DINT:= 0;  (* Codice errore sel. modo azz. *)
	CMDAZGL : BOOL:= 0;  (* Flags comando sel. azz. globale *)
	CMDMODA : BOOL:= 0;  (* Flags comando sel. modo azz. *)
	ER_AZGL : BOOL:= 0;  (* Errore su selezione azz. globale *)
	ER_MODA : BOOL:= 0;  (* Errore su selezione modo azz. *)
	OK_AZGL : BOOL:= 0;  (* Selezione azz. globale eseguita *)
	OK_MODA : BOOL:= 0;  (* Selezione modo azz. eseguita *)
	ORALLAZ : BOOL:= 0;  (* Or di tutti i tipi di azz. *)
	RUNCMD : BOOL:= 0;  (* Flags di comando in corso *)
	CMODA : CMDGEST;
	CAZGL : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* Abilitazione azzeramento globale *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando sel. azz. globale *)
	ERRCMD : BOOL:= 0;  (* Errore comando sel. azz. globale *)
  END_VAR
	
(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione selezione                 *)
(*      azzeramento globale della macchina                   *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione all' azzeramento         *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK SPEGNIM

  VAR
	CE_RBTO : DINT:= 0;  (* Codice errore reboot *)
	ER_RBTO : BOOL:= 0;  (* Errore su comando di REBOOT *)
	OK_RBTO : BOOL:= 0;  (* Comando di REBOOT eseguito *)
	CRBTO : CMDGEST;
  END_VAR

  VAR_INPUT
	ENABLE : BOOL:= 0;  (* Abilitazione comando di REBOOT *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando reboot *)
	ERRCMD : BOOL:= 0;  (* Errore comando reboot *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      reboot della macchina (shutdown)                     *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione reboot                   *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK SCALASS

  VAR
	CE_SCAL : DINT:= 0;  (* Codice Errore scalibrazione asse/i *)
	ER_SCAL : BOOL:= 0;  (* Errore su scalibrazione asse/i *)
	OK_SCAL : BOOL:= 0;  (* Fine comando scalibrazione asse/i *)
	CSCAL : CMDGEST;
  END_VAR

  VAR_INPUT
	ASSESC : DINT:= 0;  (* Numero asse da scalibrare (-1=Tutti) *)
	ENABLE : BOOL:= 0;  (* Abilitazione scalibrazione asse/i *)
  END_VAR

  VAR_OUTPUT
	ENDCMD : BOOL:= 0;  (* Fine comando azzeramento manaule *)
	ERRCMD : BOOL:= 0;  (* Errore comando azzeramento manaule *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione                           *)
(*      scalibrazione di tutti o di un asse della macchina   *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione scalibrazione asse/i     *)
(*		ASSEZM=Numero asse da scalibrare (-1=Tutti)  *)
(*	OUTPUT:	ENDCMD=Fine comando                          *)
(*		ERRCMD=Errore comando                        *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK _CMDEXE

  VAR
	ACK_R : BOOL:= 0;
	R_ACK : R_TRIG;
  END_VAR

  VAR_INPUT
	ABLE : BOOL:= 0;  (* Riflette il fronte di salita di ack_op *)
  END_VAR

  VAR_OUTPUT
	CODE : DINT:= 0;  (* Codice di ritorno dell'errore *)
	DATO0 : DINT:= 0;  (* Primo dato del codice errore *)
	DATO1 : DINT:= 0;  (* Secondo dato codice errore *)
	DATO2 : DINT:= 0;  (* Terzo dato codice errore *)
	DATO3 : DINT:= 0;  (* Quarto dato codice errore *)
	ESITO : BOOL:= 0;  (* Esito del comando inviato *)
  END_VAR

(* ------------------------------------------------------------- *)
(*      Versione 2.0                                             *)
(*      Function block che esamina il risultato del              *) 
(*      comando trasmesso al CN                                  *)
(*                                                               *)
(*	INPUT :		                                         *)
(*		ABLE    : Fronte di salita di ACK_OP             *)
(*	OUTPUT :                                                 *)
(*		COD_ERR : Codice dell'eventuale errore           *)
(*		DATA0   : Primo dato della risposta CN           *)
(*		DATA1   : Secondo dato della risposta CN         *)
(*		DATA2   : Terzo dato della risposta CN           *)
(*		DATA3   : Quarto dato della risposta CN          *)
(*		ESITO   : Esito della ricezione                  *)
(*				0 comando non finito             *)
(*				1 comando finito OK              *)
(*				2 comando finito non OK          *)
(* ------------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK _RESDIS

  VAR
	IND : DINT:= 0;  (* Indice in linee *)
	NC : DINT:= 0;  (* Nr. centro *)
  END_VAR

	(* reset variabili *)
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK _CMDTX

  VAR_INPUT
	CODICE : DINT:= 0;  (* Codice del comando da inviare *)
	DATO0 : DINT:= 0;  (* Primo dato associato al comando *)
	DATO1 : DINT:= 0;  (* Secondo dato associato al comando *)
	DATO2 : DINT:= 0;  (* Terzo dato associato al comando *)
	DATO3 : DINT:= 0;  (* Quarto dato associato al comando *)
  END_VAR

  VAR_OUTPUT
	ESITO : BOOL:= 0;  (* Risultato della trasmissione *)
  END_VAR

(* ------------------------------------------------------------- *)
(*      Versione 2.0                                             *)
(*      Function block che gestisce la trasmissione              *)
(*      di un comando al Controllo                               *)
(*                                                               *)
(*	INPUT:                                                   *)
(*		CODICE :  Codice del comando da inviare          *)
(*		DATO0  :  Primo dato associato al comando        *)
(*		DATO1  :  Secondo dato associato al comando      *)
(*		DATO2  :  Terzo dato associato al comando        *)
(*		DATO3  :  Quarto dato associato al comando       *)
(*                                                               *)
(*	OUTPUT:		                                         *)
(*		ESITO  :  Esito della trasmissione               *)
(*			  0 non ok 1 ok                          *)
(* ------------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK GESTMDI

  VAR
	FASE : DINT:= 0;  (* Fase Funzione *)
	SLOT : DINT:= 0;  (* Numero slot (0-15) *)
  END_VAR

  VAR_INPUT
	ACCX : REAL:= 0.0;  (* Percentuale accel. *)
	ANTX : REAL:= 0.0;  (* Anticipo *)
	AX : DINT:= 0;  (* Nr. interno asse *)
	CODICE : DINT:= 0;  (* Codice movimento *)
	ENABLE : BOOL:= 0;  (* Abilitazione movimento *)
	QUOTAX : REAL:= 0.0;  (* Quota target *)
	RAGGR : DINT:= 0;  (* Nr. raggruppamento (0-7) *)
	RESET : BOOL:= 0;  (* Reset posizionamento *)
	SIAC : BOOL:= 0;  (* Si accelerazione *)
	SIAN : BOOL:= 0;  (* Si anticipo *)
	SIVE : BOOL:= 0;  (* Si velocita' *)
	VELX : REAL:= 0.0;  (* Velocita' asse *)
  END_VAR

  VAR_OUTPUT
	ERROMDI : BOOL:= 0;  (* Flags di errore esec. comando *)
	FINEMDI : BOOL:= 0;  (* Flag fine comando *)
	RESULT : DINT:= 0;  (* Risultato finale *)
  END_VAR

(* --------------------------------------------------------- *)
(*      Versione 2.0                                         *)
(*      Function block di gestione MDI da PLC                *)
(*      E' in grado di pilotare un singolo asse.             *)
(*      Nel caso di comando contemporaneo di N assi si       *)
(*      dovranno utilizzare N istanze diverse tramite RAGGR  *)
(*      Governa contemporaneamente 8 assi al massimo         *)
(*      N.B. QUOTAX,VELX,ANTX e ACCX devono essere dei float *)
(*      quindi bisogna sempre mettere il "punto" !!!         *)
(*                                                           *)
(*	INPUT:  ENABLE=Abilitazione posizionamento asse      *)
(*		RESET= Reset posizionamento		     *)
(*		CODICE	0= Posizionamento assoluto           *)
(*			1= Posizionamento incrementale       *)
(*			2= Posizionamento a parcheggio       *)
(*		AX	Nr. Interno Asse (da 0 a 99)         *)
(*		RAGGR	Nr. istanza (da 0 a 7)               *)
(*		QUOTAX	Quota Obiettivo/Incrementale Asse    *)
(*		SIVE	Flag velocita'                       *)
(*		VELX	Velocita' asse (solo se SIVE=1)      *)
(*		SIAC	Flag accelerazione                   *)
(*		ACCX	Accelerazione asse (solo se SIAC=1)  *)
(*		SIAN	Flag anticipo                        *)
(*		ANTX	Anticipo asse (solo se SIAN=1)       *)
(*	OUTPUT:	FINEMDI	=1 Comando concluso                  *)
(*		ERROMDI =1 Attributo di errore esec. comando *)
(*		RESULT =risultato a comando concluso:        *)
(*			O   Okay: Fine posizionamento        *)
(*			1   Rilevato NOMOV[RAGGR]            *)
(*			2   Dati Errati in Ingresso          *)
(*			3   Raggruppamento Occupato          *)
(*			4   Raggruppamento Errato (>=8)      *)
(*			5   Okay: rilevato STOP_GRU[RAGGR]   *)
(* --------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  _REINNE

  VAR
	FEXE : BOOL:= 0;  (* Fronte salita EXE *)
	FREXE : R_TRIG;
  END_VAR

  VAR_INPUT
	NC : DINT:= 0;  (* Numero centro *)
  END_VAR

	(* Gestione reinnesco *)
	
	(* INPUT:  NC	numero centro *)
	
	
END_FUNCTION_BLOCK

FUNCTION_BLOCK CMDGEST

  VAR
	ACK_R : BOOL:= 0;
	APFASE : DINT:= 0;  (* Variabile di fase della trasmissione *)
	ERR_C : BOOL:= 0;  (* Codice di errore inviato dal CN *)
	RISULT : BOOL:= 0;  (* Esito della trasmissione *)
	R_ACK : R_TRIG;
	TX : _CMDTX;
	EXEC : _CMDEXE;
  END_VAR

  VAR_INPUT
	CODE : DINT:= 0;  (* Codice comando da inviare *)
	DATA0 : DINT:= 0;  (* Primo dato associato al comando *)
	DATA1 : DINT:= 0;  (* Secondo dato da associare al comando *)
	DATA2 : DINT:= 0;  (* Terzo dato associato comando *)
	DATA3 : DINT:= 0;  (* Quarto dato associato comando *)
	ENABLE : BOOL:= 0;  (* Abilitazione all'invio comando *)
  END_VAR

  VAR_OUTPUT
	CERR : DINT:= 0;  (* Codice errore rilevato *)
	ENDRX : BOOL:= 0;  (* Comando eseguito *)
	OUT0 : DINT:= 0;  (* Primo dato di risposta dal CN *)
	OUT1 : DINT:= 0;  (* Secondo dato di risposta del CN *)
	OUT2 : DINT:= 0;  (* Terzo dato di risposta del CN *)
	OUT3 : DINT:= 0;  (* Quarto dato di risposta dal CN *)
	RXERR : BOOL:= 0;  (* Errore esecuzione comando *)
  END_VAR

(* ------------------------------------------------------------- *)
(*      Versione 2.0                                             *)
(*      Function block di gestione comando verso il Controllo    *)
(*                                                               *)
(*	INPUT :		                                         *)
(*		ENABLE  : Abilitazione all'invio comando         *)
(*		CODE    : Codice comando                         *)
(*		DATA0   : Primo dato associato al comando        *)
(*		DATA1   : Secondo dato associato al comando      *)
(*		DATA2   : Terzo dato associato al comando        *)
(*		DATA3   : Quarto dato associato al comando       *)
(*                                                               *)
(*	OUTPUT :	                                         *)
(*		ENDRX	: Comando eseguito 		         *)
(*		RXERR   : Errore esecuzione comando              *)
(*		CERR	: Eventuale codice errore                *)
(*		OUT0    : Primo dato di ritorno CN               *)
(*		OUT1    : Secondo dato di ritorno CN             *)
(*		OUT2    : Terzo dato di ritorno CN               *)
(*		OUT3    : Quarto dato di ritorno CN              *)
(* ------------------------------------------------------------- *)

END_FUNCTION_BLOCK

FUNCTION_BLOCK  _INNESCO

  VAR
	FREXE : BOOL:= 0;  (* Fronte salita EXE *)
	FBEXE : R_TRIG;
  END_VAR

  VAR_INPUT
	NC : DINT:= 0;  (* indice centro *)
  END_VAR

	(*					*)
	(*	_INNESCO START			*)
	(*					*)
	(*	INPUT:	NC	nr. centro	*)
	(*					*)

	
END_FUNCTION_BLOCK

