###############################################################################
#
#	====== FILE: STAMPE.TCL ======
#
#	Contiene le funzioni per la stampe, ovvero:
#	1) Programma IL
#	2) Linee e Moduli I/U
#	3) Cross Reference
#	4) Indice e Copertina
#	5) Documentazione Completa
#
#	tipovis =	1 -> stampa sotto windows
#			3 -> stampa sotto unix (coi postscript)
#
#
# GG 06/12/00 Usata la variabile "ProjectConf" per contenere il nome
#             del file di configurazione.
# GG 15/01/01 Eliminati i residui riferimenti diretti a /home/edit_plc.
#
###############################################################################


#################################################################
#
#	PrimeOperazioni    - inizializzazione
#
#################################################################
proc PrimeOperazioni { } {

  global Home EditPrefs formato PrtOpt
  global Dir_ps Nomfil_debug DebugStampa NumPS NomePS
  global altezza_foglio larghezza_foglio indiceincorso
  global altez_labels largh_labels A4 NumPSbase
  global udc_usata mpp_usata
  global linea_udc linea_mpp
  global schede_speciali numero_schede_speciali
  global s24tapp num_parti_s24t

 set DebugStampa	$EditPrefs(DebugStampa)	;# SE 1 LAVORA PER DEBUG !!!!!
 set indiceincorso	1
 set NumPSbase		100
 set NumPS		$NumPSbase
 set NomePS		Plc_
 set Dir_ps		$Home/stampe/
 set Nomfil_debug	"Plc_Pagina"

 set schede_speciali(0) "udc"
 set schede_speciali(1) "mpp"
 set numero_schede_speciali 2

 set s24tapp(0) "S24T_I"
 set s24tapp(1) "S24T_O"
 set num_parti_s24t 2			;#S24T e` composto da 2x24 I/O
 #
 # setta dimensioni foglio di stampa
 #
 setta:formati:stampa
 set nomeformato $PrtOpt(prt_foglio)
 if { $nomeformato=="Custom" } {
 	set altezza_foglio $PrtOpt(prt_altfoglio)
 	set larghezza_foglio $PrtOpt(prt_larfoglio)
 } else {
 	set altezza_foglio $altez_labels($PrtOpt(prt_foglio))
 	set larghezza_foglio $largh_labels($PrtOpt(prt_foglio))
 }

 set mpp_usata [TestVarConnesse mpp 16 8]
 set udc_usata [TestVarConnesse udc 16 12]
}



#################################################################
#
#		Init:Variabili:Stampa
#
#################################################################

proc Init:Variabili:Stampa { } {

  global altezza_custom larghezza_custom formato altezza_foglio
  global indiceincorso larghezza_foglio
  global Pagina_Moduli Pagina_Riep_Moduli Pagina_CrossR
  global Pagina_ProgIL Pagina_ProgLD NumeroDellaPagina
  global tx_docall tx_index tx_stamdat tx_cnidoc tx_ulmo tx_printil
  global tx_print tx_printcov tx_printcon tx_printld tx_printcro
  global tx_ind_in tx_ind_il tx_ind_ld tx_ind_cr tx_ind_rm tx_ind_io
  global canvapp com_variabile PageWidth PageHeight
  global cornice spess_cornice data_sys 
  global desc_SI24 desc_SI8 desc_SI816 desc_SIR88
  global desc_SO24 desc_SO8 desc_SO816 desc_SOR88
  global des_input dimfogliox dimfogliox2 dimfoglioy dimfoglioy2
  global distscrtesto dist_bitmapx dist_testilinee DX_LINEASEP
  global dim_modulox dim_moduloy elenco_moduli
  global dim_moduloy_mpp dim_moduloy_udc
  global font_descr font_enorme font_grande font_initval font_lines
  global font_testi font_testibig font_titfp font_titolbold font_granden
  global font_titolino font_valore font_var font_pikkolo
  global fnt12x18 fnt5x10 fnt4x8 fnt5x7 fnt3x4 fnt4x7
  global inizio_testiy line_n Margine Margine_sx Pagex Pagey
  global matchlinesPerRiga max_num_linee max_num_moduli
  global moduli modulo modulo_n nolines nomvar
  global note_gen tx_note tx_notefin num numelin numline_riep
  global EditPrefs OK_INITIALIZE EditPrefs PrtOpt
  global outrele outst programma
  global pixelaltenorme pixelaltgrande pixelalttesto pixelalttestobig
  global pixelalttitfp pixellarenorme pixellargrande pixellartesto
  global pixellartestobig pixellartitfp
  global MezzaRiga PuntiPerRiga punti_per_mm punti_x punti_y RACK_DEF
  global ragsoc pag font_enorme2 SaltaFormFeed spaziodalfondo
  global SpesPic SpesMed SpesGra text_byte
  global tipo TipoCrossR TipoPrgIL TipoPrgLD TipoTabIO tipovis
  global titolo_riep tipo_modulo TipoIndice
  global tx_des tx_fine tx_ini tx_iol tx_lin tx_da tx_niet
  global tx_loc tx_mod tx_notfo tx_ret tx_sco tx_typ
  global versione vers_prg nomefilesenzaplc tipdat
  global flag_stampa_pil flag_stampa_pld flag_stampa_msd
  global flag_stampa_cro flag_stampa_cop flag_stampa_ind
  global NomeBitmap tcl_platform

#
# a seconda del sistema operativo decide in quale maniera stampare,
#	con i post-script oppure "direttamente"
#
 PrimeOperazioni

 if { $tcl_platform(platform) == "unix" && $tipovis==1 } {
		set tipovis 3
 }

 set canvapp       "Cni_Prova"			;# nome canvas fittizio

#
# legge la data odierna del sistema
#

 set data_odierna [clock seconds]

 set day	  [clock format $data_odierna -format %d]
 set mon	  [clock format $data_odierna -format %m]
 set year	  [clock format $data_odierna -format %y]

#
# setta le opzioni basilari
#

 set flag_stampa_pil    $PrtOpt(prt_progil)
 set flag_stampa_pld    $PrtOpt(prt_progld)
 set flag_stampa_msd    $PrtOpt(prt_tabiu)
 set flag_stampa_cro    $PrtOpt(prt_crossr)
 set flag_stampa_cop    $PrtOpt(prt_copert)
 set flag_stampa_ind    $PrtOpt(prt_indice)
 set note_gen           $PrtOpt(prt_note)
 set ragsoc             $PrtOpt(prt_ragso)
 set tipspe             $PrtOpt(prt_specorn)
 set tipdat             $PrtOpt(prt_formdata)
 set mm_Margine         $PrtOpt(prt_marsxdx)
 set mm_Margine_sx      $PrtOpt(prt_marasx)

 if { $tcl_platform(platform) == "unix" } {
 	set NomeBitmap "@$PrtOpt(prt_logo)"
 } else {
#
#	test se file bitmap esistente e con estensione BMP
#
	set estens [string toupper [file extension $PrtOpt(prt_logo)]]

	if { [file exist $PrtOpt(prt_logo) ] && $estens == ".BMP" } {
 	     set NomeBitmap $PrtOpt(prt_logo)
	} else {
 	     set NomeBitmap ""
	}
 }

 if { $tipdat=="g" } {
 	set data_sys "$day/$mon/$year"		;# EUROPEAN DATE
 } else {
 	set data_sys "$mon/$day/$year"		;# AMERICAN DATE
 }

 set tipo_spess_cornice 1			;# default - tipo spessore 1
 set cornice true				;# default - cornice presente

 if { $tipspe=="a" || $tipspe=="A" } {
	set tipo_spess_cornice 0
	set cornice false
 }

 if { $tipspe=="m" || $tipspe=="M" } {
	set tipo_spess_cornice 2
 }

 if { $tipspe=="l" || $tipspe=="L" } {
	set tipo_spess_cornice 3
 }

 if { $tipovis==1 } {
 	set mm_spaziodalfondo	15	;# spazio da lasciare per leggibilita`
 	set mm_alt_blocco	63	;# altezza disegno byte del modulo I/O
 	set mm_alt_blocco_mpp	175	;# altezza disegno modulo I/O MPP
 	set mm_alt_blocco_udc	203	;# altezza disegno modulo I/O UDC
 	set mm_lar_blocco	46	;# largh. disegno byte del modulo I/O
 } else {
 	set mm_spaziodalfondo	15	;# spazio da lasciare per leggibilita`
 	set mm_alt_blocco	54	;# altezza disegno byte del modulo I/O
 	set mm_alt_blocco_mpp	150	;# altezza disegno modulo I/O MPP
 	set mm_alt_blocco_udc	174	;# altezza disegno modulo I/O UDC
 	set mm_lar_blocco	38	;# largh. disegno byte del modulo I/O
 }

 if { $tipovis==3 } {
 	set SpesPic	1
 	set SpesMed	2
 	set SpesGra	3
 } else {
 	set SpesPic	1
 	set SpesMed	2
 	set SpesGra	3
 }

 if { $tipovis==1 } {

	#
	# via di stampa "diretta" sotto WINDOWS
	#
	set result_init [ PrtInit -d ]
        set OK_INITIALIZE [ lindex $result_init 0 ]

 	if { $OK_INITIALIZE == 0 } {
		set punti_x 1
		set punti_y 1
		set punti_per_mm 1
		set punti_per_mezmm 1
 	} else {
		set SaltaFormFeed 1
		set punti_x         [ lindex $result_init 1 ]
		set punti_y         [ lindex $result_init 2 ]
		set punti_per_mm    [ lindex $result_init 3 ]
		set punti_per_mezmm [ expr $punti_per_mm / 2 ]
 	}

	set Margine	  [expr $mm_Margine * $punti_per_mm]
	set Margine_sx	  [expr $mm_Margine_sx * $punti_per_mm]
 } else {

	set Margine		0
	set Margine_sx		0
	set punti_per_mm	1
	set punti_per_mezmm	1
 }

 set spess_cornice [expr $SpesMed + $tipo_spess_cornice - 1]

 if {$tipovis==1} {
	#
	# set dimensione moduli, numero moduli , numero linee
	#
 	set dim_modulox [expr $mm_lar_blocco*$punti_per_mm]
 	set dim_moduloy [expr $mm_alt_blocco*$punti_per_mm]
 	set dim_moduloy_mpp [expr $mm_alt_blocco_mpp*$punti_per_mm]
 	set dim_moduloy_udc [expr $mm_alt_blocco_udc*$punti_per_mm]
 	set max_num_moduli 32
 	set max_num_linee 4
	#
	# settaggio variabili per la stampa
	#
 	set pixelalttitfp	[expr $punti_per_mm * 5]  
 	set pixellartitfp	[expr $punti_per_mm * 3]  
 	set pixelalttesto	[expr $punti_per_mm * 5]  
 	set pixellartesto	[expr $punti_per_mm * 4] 
 	set pixelalttestobig	[expr $punti_per_mm * 5] 
 	set pixellartestobig	[expr $punti_per_mm * 4]
 	set pixelaltgrande	[expr $punti_per_mm * 6]
 	set pixellargrande	[expr $punti_per_mm * 4]
 	set pixelaltenorme	[expr $punti_per_mm * 6]
 	set pixellarenorme	[expr $punti_per_mm * 5]

 	set spaziodalfondo	[expr $mm_spaziodalfondo * $punti_per_mm]

	#
 	# FONTS da usare (misure calcolate in millimetri)
	#

	set font_pikkolo   "[expr 2*$punti_per_mm],[expr 3*$punti_per_mm]"
 	set font_testi     "[expr 5*$punti_per_mezmm],[expr 4*$punti_per_mm]"
 	set font_testibig  "[expr 3*$punti_per_mm],[expr 5*$punti_per_mm]"
 	set font_titfp     "[expr 2*$punti_per_mm],[expr 4*$punti_per_mm]"
 	set font_titolbold "[expr 2*$punti_per_mm],[expr 4*$punti_per_mm]"
 	set font_grande    "[expr 4*$punti_per_mm],[expr 6*$punti_per_mm]"
 	set font_enorme    "[expr 5*$punti_per_mm],[expr 6*$punti_per_mm]"

	set fnt12x18  "[expr 12*$punti_per_mm],[expr 18*$punti_per_mm]"
	set fnt5x10   "[expr 5*$punti_per_mm],[expr 10*$punti_per_mm]"
	set fnt4x8    "[expr 4*$punti_per_mm],[expr 8*$punti_per_mm]"
	set fnt4x7    "[expr 4*$punti_per_mm],[expr 7*$punti_per_mm]"
	set fnt5x7    "[expr 5*$punti_per_mm],[expr 7*$punti_per_mm]"
	set fnt3x4    "[expr 3*$punti_per_mm],[expr 4*$punti_per_mm]"

 	set font_var		$font_testibig
 	set font_descr		$font_titolbold
 	set font_initval	$font_titolbold
 	set font_valore		$font_titolbold
 	set font_titolino	$font_titolbold
 	set font_lines		$font_titolbold

 	set distscrtesto	[expr 2 * $punti_per_mm]
 	set inizio_testiy	$Margine
 	set dist_bitmapx	$punti_per_mm
 	set dist_testilinee	$punti_per_mm

	set Pagex	1	;# Distanza in cm dal bordo sx del foglio
	set Pagey	29	;# Distanza in cm tra il bordo inferiore
#				;# del foglio e l'inizio su Y del disegno

	set dimfogliox		[expr $punti_x-5]
 	set dimfoglioy		$punti_y

 	set dimfogliox2 520 	;# DIMENSIONE X DI PROVA PER IL CANVAS 
 	set dimfoglioy2 400 	;# DIMENSIONE Y DI PROVA PER IL CANVAS

	set matchlinesPerRiga		10
 	set PuntiPerRiga		[expr 6 * $punti_per_mm]
	set MezzaRiga			[expr 3 * $punti_per_mm]

	set DX_LINEASEP	[expr 50 * $punti_per_mm]

  } else {

	set pippo [expr int($altezza_foglio)]
	set pluto [expr int($larghezza_foglio)]
	set altezza_foglio $pippo
	set larghezza_foglio $pluto

	set altezza_rid		[expr $altezza_foglio-10]
	set larghezza_rid	[expr $larghezza_foglio-10]

	set Pagex	[expr ($larghezza_foglio-$larghezza_rid)/2]
	set margine_alt	[expr ($altezza_foglio-$altezza_rid)/2]
	set Pagey	[expr $margine_alt+$altezza_rid]

	set PageWidth $larghezza_rid
	set PageHeight $altezza_rid

#
#	CALCOLO LE DIMENSIONI DEL CANVAS OPPORTUNE AFFINCHE`
#	IL NUMERO DI PUNTI PER MILLIMETRO SIA UN INTERO PRECISO
#
	set dimfogliox	600 ;# dimensione base (provvisoria) del canvas su X

	set punti_per_mm [expr $dimfogliox/$larghezza_rid]
	set dimfogliox   [expr $punti_per_mm*$larghezza_rid]
	set dimfoglioy   [expr ($dimfogliox*$altezza_rid)/$larghezza_rid ]

	#
	# per stampare coi post-script un piccolo margine c'e` comunque
	#

	if { $mm_Margine<5 } {
		set mm_Margine 0
	} else {
		incr mm_Margine -5
	}

	set Margine	  [expr $mm_Margine * $punti_per_mm]
	set Margine_sx	  [expr $mm_Margine_sx * $punti_per_mm]
	set spess_cornice [expr $SpesMed + $tipo_spess_cornice - 1]

	set punti_x	$dimfogliox
	set punti_y	$dimfoglioy

 	set dimfogliox2	540	;# Dimensione di Prova del Canvas per visualiz.
 	set dimfoglioy2	380 	;# Dimensione di Prova del Canvas per visualiz.

 	set PuntiPerRiga [expr 5 * $punti_per_mm]
	set MezzaRiga    [expr $PuntiPerRiga/2]

	#
	# set dimensione moduli, numero moduli , numero linee
	#

 	set dim_modulox [expr $mm_lar_blocco*$punti_per_mm]
 	set dim_moduloy [expr $mm_alt_blocco*$punti_per_mm]
 	set dim_moduloy_mpp [expr $mm_alt_blocco_mpp*$punti_per_mm]
 	set dim_moduloy_udc [expr $mm_alt_blocco_udc*$punti_per_mm]

 	set max_num_moduli 32
 	set max_num_linee 4

 	set spaziodalfondo	[expr $mm_spaziodalfondo * $punti_per_mm]

	#
	#	SETTAGGIO FONT DA UTILIZZARE
	#	CALCOLO DELLA OCCUPAZIONE SPAZIO PER OGNI FONT
	#

	set courier_gras	"-adobe-courier-bold-r-normal"
	set courier_norm	"-adobe-courier-medium-r-normal"

	set fattore_molt	$punti_per_mm

	#
	# ptsz / 10 = punti di canvas
	#
	set ptsz 100
	set mmfn [expr $ptsz/50]

	set font_pikkolo	"$courier_norm--*-$ptsz-*-*-*-*-*-*"
 	set font_titfp		"$courier_norm--*-$ptsz-*-*-*-*-*-*"
 	set font_titolino	"$courier_norm--*-$ptsz-*-*-*-*-*-*"
 	set font_lines		"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_testipic	"$courier_norm--*-$ptsz-*-*-*-*-*-*"
 	set font_descr		"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_initval	"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_testi		"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_titolbold	"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_valore		"$courier_gras--*-$ptsz-*-*-*-*-*-*"

 	set pixelalttitfp [expr $mmfn*$fattore_molt]
 	set pixellartitfp $pixelalttitfp
 	set pixelalttesto [expr $mmfn*$fattore_molt]
 	set pixellartesto $pixelalttesto

	incr fattore_molt

	set ptsz 150
	set mmfn [expr $ptsz/50]

 	set font_testibig	"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set font_granden	"$courier_norm--*-$ptsz-*-*-*-*-*-*"
 	set font_var		"$courier_gras--*-$ptsz-*-*-*-*-*-*"

 	set pixelalttestobig [expr $mmfn*$fattore_molt]
 	set pixellartestobig $pixelalttestobig

	incr fattore_molt

	set ptsz 200
	set mmfn [expr $ptsz/50]

 	set font_grande		"$courier_gras--*-$ptsz-*-*-*-*-*-*"

 	set pixelaltgrande [expr $mmfn*$fattore_molt]
 	set pixellargrande $pixelaltgrande

	set ptsz 250
	set mmfn [expr $ptsz/50]

 	set font_enorme		"$courier_gras--*-$ptsz-*-*-*-*-*-*"
 	set pixelaltenorme [expr $mmfn*$fattore_molt]
 	set pixellarenorme $pixelaltenorme

	set ptsz 350
	set mmfn [expr $ptsz/50]

 	set font_enorme2	"$courier_gras--*-$ptsz-*-*-*-*-*-*"

 	set pixelaltenorme2 [expr $mmfn*$fattore_molt]
 	set pixellarenorme2 $pixelaltenorme2

	incr fattore_molt

 	# VARIABILI GENERICHE 
 	# set distscrtesto 4
 	# set inizio_testiy 8
 	# set dist_bitmapx 8
 	# set dist_testilinee 4

 	set distscrtesto	[expr 2 * $punti_per_mm]
 	set inizio_testiy	$Margine
 	set dist_bitmapx	$punti_per_mm
 	set dist_testilinee	$punti_per_mm

	set matchlinesPerRiga	10

	set DX_LINEASEP	[expr 42 * $punti_per_mm]

 }	;# FINE DI INIZIALIZZAZIONE PER SOLA VISUALIZZAZIONE

 # TESTI UTILIZZATI NEL TITOLO DI FINE PAGINA

 set tx_cnidoc     [GetString prt_stam_cnido]
 set tx_stamdat    [GetString prt_stam_stamd]

 set tx_docall     [GetString prt_stam_docum]
 set tx_index      [GetString prt_stam_index]
 set tx_print      [GetString prt_stam_print]
 set tx_printcov   [GetString prt_stam_pcove]
 set tx_printcon   [GetString prt_stam_pconn]
 set tx_printld    [GetString prt_stam_pldpr]
 set tx_printcro   [GetString prt_stam_pcros]
 set tx_printil    [GetString prt_stam_pilpr]

 set tx_ind_in     [GetString prt_stam_in_in]
 set tx_ind_il     [GetString prt_stam_in_il]
 set tx_ind_ld     [GetString prt_stam_in_ld]
 set tx_ind_cr     [GetString prt_stam_in_cr]
 set tx_ind_rm     [GetString prt_stam_in_rm]
 set tx_ind_io     [GetString prt_stam_in_io]

 set programma     [GetString prt_stam_tprog]
 set versione      [GetString prt_stam_tvers]
 set tipo          [GetString prt_stam_ttipo]
 set pag           [GetString prt_stam_npage]

 set TipoPrgIL     [GetString prt_stam_ar_il]
 set TipoPrgLD     [GetString prt_stam_ar_ld]
 set TipoCrossR    [GetString prt_stam_ar_cr]
 set TipoTabIO     [GetString prt_stam_ar_io]
 set TipoIndice    [GetString prt_stam_ar_in]

 set modulo_n      [GetString prt_stam_modnu]
 set line_n        [GetString prt_stam_linen]
 set tipo_modulo   [GetString prt_stam_tipmo]
 set moduli        [GetString prt_stam_modu2]
 set modulo        [GetString prt_stam_modu1]
 set num           [GetString prt_stam_nume1]

 set desc_SOR88    [GetString prt_stam_SOR88]
 set desc_SO24     [GetString prt_stam_SO24]
 set desc_SO816    [GetString prt_stam_SO816]
 set desc_SO8      [GetString prt_stam_SO8]
 set desc_SIR88    [GetString prt_stam_SIR88]
 set desc_SI24     [GetString prt_stam_SI24]
 set desc_SI816    [GetString prt_stam_SI816]
 set desc_SI8      [GetString prt_stam_SI8]

 set text_byte     [GetString prt_stam_tbyt]
 set des_input     [GetString prt_stam_8inp]
 set outst         [GetString prt_stam_8sou]
 set outrele       [GetString prt_stam_8rou]

 set titolo_riep   [GetString prt_stam_trie]
 set numline_riep  [GetString prt_stam_nrie]

 set tx_ulmo       [GetString prt_stam_dulmo]

 set tx_niet       [GetString prt_stam_no]
 set tx_da         [GetString prt_stam_si]
 set tx_note       [GetString prt_stam_xnot]
 set tx_notefin    [GetString prt_stam_xnot]
 set tx_lin        [GetString prt_stam_xlin]
 set tx_typ        [GetString prt_stam_xtyp]
 set tx_sco        [GetString prt_stam_xsco]
 set tx_ret        [GetString prt_stam_xret]
 set tx_loc        [GetString prt_stam_xloc]
 set tx_mod        [GetString prt_stam_xmod]
 set tx_iol        [GetString prt_stam_xiol]
 set tx_ini        [GetString prt_stam_xini]
 set tx_des        [GetString prt_stam_xdes]
 set tx_notfo      [GetString prt_stam_xnof]
 set tx_fine       [GetString prt_stam_xfin]
}


##################################################################
#
#	crea:postscript
#
#		funzione comune di generazione del file
#		postscript da un canvas
#
##################################################################
proc crea:postscript { xCanvas psname } {

  global	indiceincorso dimfogliox dimfoglioy Dir_ps PageHeight
  global	Pagey Pagex PageWidth DebugStampa NumPS NomePS docall

	if { $DebugStampa==1 } {
		#puts stdout "Creating Postscript $NumPS"
	} else {
		$xCanvas postscript -file "$Dir_ps$NomePS$NumPS.ps"\
			-width $dimfogliox -height $dimfoglioy\
			-pagewidth $PageWidth.m\
			-pageheight $PageHeight.m\
			-pageanchor nw\
			-pagex $Pagex.m -pagey $Pagey.m

		if { $docall==0 } {
		  set kom1 "catch { exec lpr $Dir_ps$NomePS$NumPS.ps } app1"
		  eval $kom1
		  set kom2 "catch { exec rm $Dir_ps$NomePS$NumPS.ps } app2"
		  eval $kom2
		}
	}

	incr NumPS
}


#################################################################
#								#
# CREA:TITOLO:FINEPAG - crea titolo di "fine pagina"		#
#								#
#################################################################

proc crea:titolo:finepag {canv tipoStp}	{

	global	indiceincorso
	global	SpesMed NomeBitmap tx_notefin y_cnidoc tx_cnidoc
	global	xiniz_titfp tipovis dist_bitmapx dimbitmapy
	global	cornice y_titolofp dimfogliox dimfoglioy distscrtesto
	global	versione programma tipo note_gen data_sys tx_stamdat
	global	pag pixelalttitfp dist_testilinee PuntiPerRiga
	global	pixellartitfp font_titfp vers_prg font_testi MezzaRiga
	global	ragsoc xi_pagina xf_pagina yf_pagina punti_per_mm
 	global	nomefilesenzaplc NumeroDellaPagina font_pikkolo

 #
 # CALCOLA POSIZIONE DEL TITOLO DI FINE PAGINA
 #

 set xiniz_titfp  $xi_pagina
 set xfin_titfp   $xf_pagina
 set yfin_titfp   $yf_pagina

 set dimbitmapx [expr 18*$punti_per_mm] 	;# dimensione X del bitmap
 set dimbitmapy [expr 18*$punti_per_mm] 	;# dimensione Y del bitmap

 set dim_datasys 10	;# numero massimo di car. per la data di sistema
 set car_numpag 4	;# numero massimo di car. per il numero pagina

 #
 # Traccia Rettangolo del Titolo di Fine Pagina
 #
 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xiniz_titfp,$y_titolofp,$xfin_titfp,$y_titolofp
	}
 } else {
	$canv create line $xiniz_titfp $y_titolofp $xfin_titfp $y_titolofp
 }

 if {$cornice == "false"} {
     if { $tipovis==1 } {
	if { $indiceincorso==1 } {
   	PrtLine -S$SpesMed -P$xiniz_titfp,$y_titolofp,$xiniz_titfp,$yfin_titfp
   	PrtLine -S$SpesMed -P$xfin_titfp,$y_titolofp,$xfin_titfp,$yfin_titfp
   	PrtLine -S$SpesMed -P$xiniz_titfp,$yfin_titfp,$xfin_titfp,$yfin_titfp
	}
     } else {
   	$canv create line $xiniz_titfp $y_titolofp $xiniz_titfp $yfin_titfp
   	$canv create line $xfin_titfp $y_titolofp $xfin_titfp $yfin_titfp
   	$canv create line $xiniz_titfp $yfin_titfp $xfin_titfp $yfin_titfp
     }
 }

 #
 # Disegna Bitmap per LOGO
 #

 set xlinea_logo  [ expr $xiniz_titfp + $dimbitmapx ]
 set dist_bitmapy $dist_bitmapx

 if { $tipovis==3 } {
 	set xbitmap  [expr $xiniz_titfp+$punti_per_mm]
 	set ybitmap  [expr $y_titolofp+$punti_per_mm]
 	set xfbitmap [expr $xbitmap+$dimbitmapx-2*$punti_per_mm]
 	set yfbitmap [expr $ybitmap+$dimbitmapy-2*$punti_per_mm]
	$canv create bitmap $xbitmap $ybitmap -bitmap $NomeBitmap -anchor nw
 }

 if { $tipovis==1 } {
 	set xbitmap  [expr $xiniz_titfp+$punti_per_mm]
 	set ybitmap  [expr $y_titolofp+$punti_per_mm]
 	set xfbitmap [expr $xbitmap+$dimbitmapx-2*$punti_per_mm]
 	set yfbitmap [expr $ybitmap+$dimbitmapy-2*$punti_per_mm]
	if { $indiceincorso==1 && $NomeBitmap != "" } {
	  PrtBmp $NomeBitmap -C$xbitmap,$ybitmap,$xfbitmap,$yfbitmap
	}
 }

 #
 # Linea per la separazione del "LOGO"
 #

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xlinea_logo,$y_titolofp,$xlinea_logo,$yfin_titfp
	}
 } else {
	$canv create line $xlinea_logo $y_titolofp $xlinea_logo $yfin_titfp
 }

 #
 # Linee orizzontali
 #

 set yprimal_oriz [expr $y_titolofp + $PuntiPerRiga]
 set yseconl_oriz [expr $y_titolofp + 2*$PuntiPerRiga]

 if { $tipovis==1 } {
	set xlogo $xlinea_logo
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xlogo,$yprimal_oriz,$xfin_titfp,$yprimal_oriz
	PrtLine -S$SpesMed -P$xlogo,$yseconl_oriz,$xfin_titfp,$yseconl_oriz
	}
 } else {
	$canv create line $xlinea_logo $yprimal_oriz $xfin_titfp $yprimal_oriz
	$canv create line $xlinea_logo $yseconl_oriz $xfin_titfp $yseconl_oriz
 }

 #
 # Linea Verticale di Separazione Programma / Versione
 #

 set x_prover [expr $xlinea_logo + ((($xfin_titfp - $xlinea_logo)/5)*3) ]

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$x_prover,$y_titolofp,$x_prover,$yprimal_oriz
	}
 } else {
	$canv create line $x_prover $y_titolofp $x_prover $yprimal_oriz
 }

 #
 # Testi per la Versione e per il Programma
 #

 if { $tipovis==1 } {
 	set y_vers_prog  [expr $y_titolofp+$dist_testilinee]
 } else {
 	set y_vers_prog  [expr $y_titolofp+$MezzaRiga]
 }

 set app [string length $vers_prg]	;# leva gli apicini dalla versione
 set app1 [expr $app-2]
 set app2 [string range $vers_prg 1 $app1]

 set testo "$versione $app2"
 crea:testo:bold $canv [expr $x_prover+$distscrtesto]\
					$y_vers_prog $testo $font_titfp
 set testo "$programma $nomefilesenzaplc"
 crea:testo:bold $canv [expr $xlinea_logo+$distscrtesto]\
	     				$y_vers_prog $testo $font_titfp

 #
 # Linea di Separazione Note / Tipo
 #

 set x_prover [expr $xlinea_logo + ((($xfin_titfp - $xlinea_logo)/5)*3)]

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$x_prover,$yprimal_oriz,$x_prover,$yseconl_oriz
	}
 } else {
	$canv create line $x_prover $yprimal_oriz $x_prover $yseconl_oriz
 }

 #
 # Testi per il Tipo
 #

 if { $tipovis==1 } {
 	set y_note_tipo  [expr $yprimal_oriz+$dist_testilinee]
 } else {
 	set y_note_tipo  [expr $yprimal_oriz+$MezzaRiga]
 }

 set testo "$tipo $tipoStp"
 crea:testo:bold $canv [expr $x_prover+$distscrtesto]\
           				$y_note_tipo $testo $font_titfp

 set testo "$tx_notefin $note_gen"
 if { $tipovis==1 } {
	crea:testo:short:bold $canv [expr $xlinea_logo+$distscrtesto]\
             		$y_note_tipo $testo $font_titfp $x_prover
 } else {
 	crea:testo:bold $canv [expr $xlinea_logo+$distscrtesto]\
             				$y_note_tipo $testo $font_titfp
 }


 #
 # Linea Verticale di Separazione Data / Ragione Sociale
 #
 if { $tipovis==1 } {
 	set x_datars [expr $xlinea_logo+55*$punti_per_mm]
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$x_datars,$yseconl_oriz,$x_datars,$yfin_titfp
	}
 } else {
	set x_datars [expr $xlinea_logo+55*$punti_per_mm]
	$canv create line $x_datars $yseconl_oriz $x_datars $yfin_titfp
 }

 #
 # SCRITTURA DATA DI STAMPA
 #
 set tesdat "$tx_stamdat $data_sys"

 if { $tipovis==1 } {
	crea:testo:short:bold $canv [expr $xlinea_logo+$distscrtesto]\
           	[expr $yseconl_oriz+$dist_testilinee]\
		$tesdat $font_titfp $x_datars
 } else {
	crea:testo:bold $canv [expr $xlinea_logo+$distscrtesto]\
           	[expr $yseconl_oriz+$MezzaRiga] $tesdat $font_titfp
 }

 #
 # LINEA SEPARAZIONE RAGIONE SOCIALE / NUMERO PAGINA
 #

 if { $tipovis==1 } {
 	set x_rspagina [expr $xfin_titfp-25*$punti_per_mm]
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$x_rspagina,$yseconl_oriz,$x_rspagina,$yfin_titfp
	}
 } else {
 	set x_rspagina [expr $xfin_titfp-25*$punti_per_mm]
	$canv create line $x_rspagina $yseconl_oriz $x_rspagina $yfin_titfp
 }

 #
 # SCRITTURA RAGIONE SOCIALE
 #

 if { $tipovis==1 } {
 	set inizio_testo [expr $x_datars+$distscrtesto]
	crea:testo:short:bold $canv $inizio_testo\
		[expr $yseconl_oriz+$dist_testilinee]\
		$ragsoc $font_titfp $x_rspagina
 } else {
 	set inizio_testo [expr $x_datars+$dist_testilinee]
 	scrivi:testo $canv $inizio_testo [expr $yseconl_oriz+$MezzaRiga]\
             $ragsoc $font_titfp [expr $x_rspagina-$x_datars]
 }

 #
 # SCRITTURA DEL NUMERO DELLA PAGINA
 #

 incr NumeroDellaPagina
 set testo "$pag $NumeroDellaPagina"

 if { $tipovis==1 } {
 	crea:testo:bold $canv [expr $x_rspagina+$distscrtesto]\
           [expr $yseconl_oriz+$dist_testilinee] $testo $font_titfp
 } else {
 	crea:testo:bold $canv [expr $x_rspagina+$distscrtesto]\
           [expr $yseconl_oriz+$MezzaRiga] $testo $font_titfp
 }

 crea:testo:bold $canv $xiniz_titfp $y_cnidoc $tx_cnidoc $font_pikkolo
} ;# FINE CREA:TITOLO:FINEPAG


##
## FINE:CROSSR - disegna la cornicetta di fine cross-reference
## -----------------------------------------------------------
##

proc fine:crossr { canv y_blocco } {

	global	indiceincorso
	global	SpesMed dist_testilinee PuntiPerRiga
	global	xi_pagina xf_pagina posx_testo tx_fine pixelaltgrande
	global	PuntiPerRiga font_titolino font_grande pixellargrande
	global	distscrtesto cornice pixelalttesto spaziodalfondo tipovis


   if { $tipovis==1 } {
   	set yi [expr $y_blocco+$dist_testilinee*4]
   	set posy [expr $yi+$dist_testilinee]
   	crea:testo:centrato:bold\
			 $xi_pagina $xf_pagina $posy $tx_fine $font_grande

   } else {
   	set yi [expr $y_blocco+$pixelalttesto/2]
   	set xi [posiz:testo:centrato $xi_pagina $xf_pagina $tx_fine\
          	$pixellargrande]
   	set posy [expr $yi+$pixelaltgrande/2]
   	crea:testo $canv $xi $posy $tx_fine $font_grande
   }
   
   if { $tipovis==1 } {

        set posy [expr $posy+$PuntiPerRiga]
	if { $indiceincorso==1 } {
   	PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
   	PrtLine -S$SpesMed -P$xi_pagina,$yi,$xf_pagina,$yi
	}

   } else {

	set posy [expr $posy+$distscrtesto+$pixelaltgrande/2]
   	$canv create line $xi_pagina $posy $xf_pagina $posy
   	$canv create line $xi_pagina $yi $xf_pagina $yi
   }

   if {$cornice == "false" } {		
	if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$posy,$xi_pagina,$yi
		PrtLine -S$SpesMed -P$xf_pagina,$yi,$xf_pagina,$posy
		}
	} else {
		$canv create line $xi_pagina $posy $xi_pagina $yi
		$canv create line $xf_pagina $yi $xf_pagina $posy
	}
   }

   return $posy
}

##
##
## INIZIO:CROSSR - disegna la cornicetta di inizio cross-reference
## ---------------------------------------------------------------
##

proc inizio:crossr { canv } {

	global	indiceincorso
 	global	PuntiPerRiga yprima_lin cornice pixelaltgrande dist_testilinee
 	global	posx_testo tx_note yi_pagina font_var SpesMed MezzaRiga
 	global	xi_pagina xf_pagina nomefilesenzaplc font_titfp
 	global	xi_scrit font_grande tipovis punti_per_mm pixellargrande

 if { $tipovis==1 || $tipovis==3 } {

 	set yrett_note	[expr $yprima_lin+2*$PuntiPerRiga]
 	set yf_lin	[expr $yrett_note+$PuntiPerRiga]

	if { $tipovis==1 } {
 		set posy [expr $yprima_lin+$dist_testilinee*2]
 		crea:testo:centrato:bold $xi_pagina $xf_pagina $posy\
					$nomefilesenzaplc $font_grande
	} else {
		set xi [posiz:testo:centrato $xi_pagina $xf_pagina\
					$nomefilesenzaplc $pixellargrande]
 		set posy [expr $yprima_lin+$PuntiPerRiga]
 		crea:testo $canv $xi $posy $nomefilesenzaplc $font_grande
	}
   
 	set posy $yrett_note

	if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
		}
   		set posy [expr $posy+$dist_testilinee]
	} else {
		$canv create line $xi_pagina $posy $xf_pagina $posy
   		set posy [expr $posy+$MezzaRiga]
	}

   	set testo "$tx_note"
   	crea:testo $canv $xi_scrit $posy $testo $font_titfp

   	set posy $yf_lin

	if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
		}
	} else {
		$canv create line $xi_pagina $posy $xf_pagina $posy
	}

   	if {$cornice == "false" } {		
		if { $tipovis==1 } {
			if { $indiceincorso==1 } {
			PrtLine -S$SpesMed\
				-P$xi_pagina,$yi_pagina,$xf_pagina,$yi_pagina
			PrtLine -S$SpesMed\
				-P$xi_pagina,$yi_pagina,$xi_pagina,$posy
			PrtLine -S$SpesMed\
				-P$xf_pagina,$yi_pagina,$xf_pagina,$posy
			}
		} else {
			$canv create line\
				xi_pagina $yi_pagina $xf_pagina $yi_pagina
			$canv create line\
				xi_pagina $yi_pagina $xi_pagina $posy
			$canv create line\
				$xf_pagina $yi_pagina $xf_pagina $posy
		}
   	}

 	return $posy

 } else {
 	return $yprima_lin
 }
}


##
## CREA:TRATTEGGIO - crea una linea tratteggiata (step = lunghezza trattino)
##
proc crea:tratteggio {canv xinizio yappo step} {

	global	indiceincorso xf_pagina tipovis

	for {set xa $xinizio} {$xa <$xf_pagina} {incr xa [expr $step*2]} {
		set xf [expr $xa+$step]
		if { $xf > $xf_pagina } {
			set xf $xf_pagina
		}
		if { $tipovis==1 } {
			if { $indiceincorso==1 } {
			PrtLine -P$xa,$yappo,$xf,$yappo -S1
			}
		} else {
			$canv create line $xa $yappo $xf  $yappo
		}
 	}
}

##
## CREA:CORNICE - crea una cornice alla zona descritta
##
proc crea:cornice {canv posix posiy posfx posfy } {

	global	indiceincorso spess_cornice tipovis

 if { $tipovis==1 } {
	set xi $posix
	set xf $posfx
	set yi $posiy
	set yf $posfy
	if { $indiceincorso==1 } {
	PrtPoli -S$spess_cornice -P$xi,$yi,$xf,$yi,$xf,$yf,$xi,$yf
	}
 } else {
	$canv create rect $posix $posiy $posfx $posfy -width $spess_cornice
 }
}

#############################################################################
##
## COORD:PAGINA
## ------------
##
## calcola le coordinate della pagina per il disegno della cornice 
## oppure per la scrittura dei testi (se cornice non presente). 
## calcola le coordinate della pagina da cui si puo' iniziare a scrivere.
##
#############################################################################

proc coord:pagina { dimx dimy } {

	global	indiceincorso cornice Margine Margine_sx inizio_testiy tipovis
	global	distscrtesto xi_pagina xf_pagina yi_pagina yf_pagina 
	global	xi_scrit yi_scrit xf_scrit yf_scrit MezzaRiga
	global	dx_pagina dy_pagina tipovis spess_cornice punti_per_mm
	global	y_cnidoc PuntiPerRiga dist_testilinee

#
# calcolo le dimensioni della pagina 
#

 set xi_pagina [expr $Margine+$Margine_sx]
 set xf_pagina [expr $dimx-$Margine]
 set yi_pagina $inizio_testiy
 set yf_pagina [expr $dimy-$inizio_testiy]

 if { $tipovis==1 } {
	set yf_pagina [expr $yf_pagina-4*$punti_per_mm]
	set y_cnidoc  [expr $yf_pagina+1*$punti_per_mm]
 } else {
	set yf_pagina [expr $yf_pagina-$PuntiPerRiga]
	set y_cnidoc  [expr $yf_pagina+$MezzaRiga]
 }

#
#	Calcolo coordinate per scrittura dei testi
#

 if {$cornice} {

   set spesscor $spess_cornice

   set xi_scrit [expr $xi_pagina+$distscrtesto+$spesscor]
   set yi_scrit [expr $yi_pagina+$distscrtesto+$spesscor]
   set xf_scrit [expr $xf_pagina-$distscrtesto-$spesscor]
   set yf_scrit [expr $yf_pagina-$distscrtesto-$spesscor]

 } else {

   set xi_scrit $xi_pagina
   set yi_scrit $yi_pagina
   set xf_scrit $xf_pagina
   set yf_scrit $yf_pagina

 }

 set dx_pagina [ expr $xf_pagina - $xi_pagina ]
 set dy_pagina [ expr $yf_pagina - $yi_pagina ]
}


##
## POSIZ:TESTO:CENTRATO - calcola la coord. X per scrittura "centrata"
##
proc posiz:testo:centrato {xiniz xfine testo dimencar} {
 return [expr $xiniz+($xfine-$xiniz-[string length $testo]*$dimencar)/2]
}


##
## POSIZ:TITOLO:FINEPAG - calcola posizione per titolo di fine pagina
##
proc posiz:titolo:finepag {} {

	global y_titolofp yf_pagina PuntiPerRiga

 set y_titolofp [expr $yf_pagina-$PuntiPerRiga*3]
}
   

##
## SCRIVI:TESTO - scrive testo troncandolo se troppo lungo
##
proc scrivi:testo {canv posx posy testo font_usato lungh_max} {

 	global	pixellartitfp pixellartesto pixellartestobig pixellargrande
 	global	pixellarenorme font_titfp font_titolbold font_testi font_grande
	global	indiceincorso font_enorme font_testibig font_initval tipovis

 # in base al font, setto la dimensione del carattere

 set dim_car $pixellartesto

 if {$font_usato == $font_titfp} {
	set dim_car $pixellartitfp
 }
 if {$font_usato == $font_titolbold} {	
	set dim_car $pixellartitfp
 }
 if {$font_usato == $font_testi} {	
	set dim_car $pixellartesto
 }
 if {$font_usato == $font_initval} {	
	set dim_car $pixellartesto
 }
 if {$font_usato == $font_testibig} {	
	set dim_car $pixellartestobig
 }
 if {$font_usato == $font_grande} {	
	set dim_car $pixellargrande
 }
 if {$font_usato == $font_enorme} {	
	set dim_car $pixellarenorme
 }

 set testap $testo
 crea:testo $canv $posx $posy $testap $font_usato
}


#
# CREA:TESTO:BOLD - scrive il testo BOLD senza controllo lunghezza
#
proc crea:testo:bold {canv posx posy testo font_testo } {

	global	indiceincorso xf_pagina tipovis pixelaltgrande dist_testilinee
  	global	punti_per_mm

  set xf [expr $xf_pagina - 3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app1 [expr $posy + 0]
	set y_app2 [expr $posy + 4*$pixelaltgrande]
	if { $indiceincorso==1 } {
	PrtText $testo -A0 -B -H$font_testo -P$posx,$y_app1,$xf,$y_app2
	}
  } else {
	$canv create text $posx $posy\
		-anchor w -text $testo -font $font_testo
  }
}

#
# CREA:TESTO:SHORT:BOLD - scrive il testo BOLD con controllo lunghezza
#
proc crea:testo:short:bold {canv posx posy testo font_testo xfinale } {

	global	indiceincorso
  	global	xf_pagina tipovis pixelaltgrande
  	global	dist_testilinee punti_per_mm

  set xf [expr $xfinale-3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app1 [expr $posy + 0]
	set y_app2 [expr $posy+4*$pixelaltgrande]
	if { $indiceincorso==1 } {
	PrtText $testo -A0 -B -H$font_testo -P$posx,$y_app1,$xf,$y_app2
	}
  } else {
	$canv create text $posx $posy\
		-anchor w -text $testo -font $font_testo
  }
}


#
# CREA:TESTO:SHORT - scrive il testo con controllo lunghezza
#
proc crea:testo:short {canv posx posy testo font_testo xfinale } {

	global	indiceincorso
  	global	xf_pagina tipovis pixelaltgrande
  	global	dist_testilinee punti_per_mm

  set xf [expr $xfinale-3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app2 [expr $posy+4*$pixelaltgrande]
	set y_app1 [expr $posy + 0]
	PrtText $testo -A0 -H$font_testo -P$posx,$y_app1,$xf,$y_app2
  } else {
	$canv create text $posx $posy\
		-anchor w -text $testo -font $font_testo
  }
}


#
# CREA:TESTO - scrive testo senza controllo lunghezza
#
proc crea:testo {canv posx posy testo font_testo } {

	global	indiceincorso
  	global	xf_pagina tipovis pixelaltgrande dist_testilinee
  	global	punti_per_mm

  set xf [expr $xf_pagina - 3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app2 [expr $posy+4*$pixelaltgrande]
	set y_app1 [expr $posy + 0]
	if { $indiceincorso==1 } {
	PrtText $testo -A0 -H$font_testo -P$posx,$y_app1,$xf,$y_app2
	}
  } else {
	$canv create text $posx $posy\
		-anchor w -text $testo -font $font_testo
  }
}


#
# CREA:TESTO:CENTRATO:BOLD - scrive testo BOLD centrato (solo se tipovis==1)
#
proc crea:testo:centrato:bold { xi xfin posy testo font_testo } {

	global	indiceincorso tipovis pixelaltgrande punti_per_mm

  set xf [expr $xfin - 3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app2 [expr $posy+4*$pixelaltgrande]
	set y_app1 [expr $posy + 0]
	if { $indiceincorso==1 } {
	PrtText $testo -A1 -B -H$font_testo -P$xi,$y_app1,$xf,$y_app2
	}
  }
}


#
# CREA:TESTO:CENTRATO - scrive testo centrato (solo se tipovis==1)
#
proc crea:testo:centrato { xi xfin posy testo font_testo } {

	global	indiceincorso tipovis pixelaltgrande punti_per_mm

  set xf [expr $xfin - 3*$punti_per_mm]

  if { $tipovis==1 } {
	set y_app2 [expr $posy+4*$pixelaltgrande]
	set y_app1 [expr $posy + 0]
	if { $indiceincorso==1 } {
	PrtText $testo -A1 -H$font_testo -P$xi,$y_app1,$xf,$y_app2
	}
  }
}


##
##
## CONTA:MODULI - conta il numero di moduli nella linea 
## ----------------------------------------------------
##
proc conta:moduli { numero_linea } {

	global	indiceincorso SpesMed max_num_moduli elenco_moduli

 set numero_mod 0

 for {set i 0} { $i< $max_num_moduli } {incr i} {
	if [info exist elenco_moduli($numero_linea,$i)] {
		if { $elenco_moduli($numero_linea,$i) != -1 } {
			incr numero_mod
		}
	}
 }

 return $numero_mod
}


##
## MODULO:SCHEMATIZZ
## =================
##
## NOTE: Disegna il modulo schematizzato.
##       Il parametro "rett_reverse" indica quale rettangolo annerire
##       Il parametro "num_rett" indica quanti rettangoli disegnare
##
proc modulo:schematizz {canv xi_modulo yi_modulo rett_reverse num_rett} {

	global	indiceincorso SpesMed dim_moduloy tipovis

 set dimenx_rettang 7	;# dimensione x del rettangolo per schematizz. modulo
 set dimeny_rettang 22	;# dimensione y del rettangolo per schematizz. modulo

 set d_moduleret 5	;# distanza x e y tra modulo e rettangolo
 set d_retainret 20	;# distanza x e y tra rettangolo e rettangolo
 set d_retaincor 3	;# distanza x e y tra rettangolo e cornice

# calcolo la posizione del primo rettangolo in alto a sinistra

 set posy_dis [expr $yi_modulo+$dim_moduloy/2-$dimeny_rettang]
 set posx_dis [expr $xi_modulo-$d_moduleret-$d_retainret-2*$dimenx_rettang]

#
# crea la cornice di contorno ai rettangoli
#

 if { $tipovis==1 } {
   	set pxi [expr $posx_dis-$d_retaincor]
   	set pyi [expr $posy_dis-$d_retaincor]
   	set pxf [expr $posx_dis+2*$dimenx_rettang+$d_retaincor+$d_moduleret]
   	set pyf [expr $posy_dis+$d_moduleret+$d_retaincor+2*$dimeny_rettang]
	if { $indiceincorso==1 } {
   	PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
	}
 } else {
   	$canv create rect\
   	[expr $posx_dis-$d_retaincor]\
	[expr $posy_dis-$d_retaincor]\
   	[expr $posx_dis+2*$dimenx_rettang+$d_retaincor+$d_moduleret]\
   	[expr $posy_dis+$d_moduleret+$d_retaincor+2*$dimeny_rettang]
 }

# creo i tre rettangoli

 if {$rett_reverse == 0} {	;# creo il rett.0 e lo riempio 

   	if { $tipovis==1 } {

		set pxi $posx_dis
		set pyi $posy_dis
   		set pxf [expr $posx_dis+$dimenx_rettang]
		set pyf [expr $posy_dis+$dimeny_rettang]
		if { $indiceincorso==1 } {
   		PrtPoli -S$SpesMed -F1 -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}

   	} else {

   		$canv create rect\
   		$posx_dis $posy_dis\
   		[expr $posx_dis+$dimenx_rettang]\
		[expr $posy_dis+$dimeny_rettang] -fill black
   	}

 } else {			;# creo il rett.0 vuoto

   	if { $tipovis==1 } {
   		set pxf [expr $posx_dis+$dimenx_rettang]
		set pyf [expr $posy_dis+$dimeny_rettang]
		set pxi $posx_dis
		set pyi $posy_dis
		if { $indiceincorso==1 } {
   		PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}
   	} else {
   		$canv create rect\
   		$posx_dis $posy_dis\
   		[expr $posx_dis+$dimenx_rettang]\
		[expr $posy_dis+$dimeny_rettang]
   	}
 }

 if {$num_rett == 2} {		;# disegno 1 rettangolo piccolo e 1 piu' lungo

   if {$rett_reverse == 12} {	;# rettangolo piu' lungo (out rele) pieno

	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
		set pyi $posy_dis
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$d_moduleret+2*$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -F1 -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}

	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret] $posy_dis\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$d_moduleret+2*$dimeny_rettang]\
      		-fill black
	}
   } else {			;# rettangolo piu' lungo (out rele) vuoto
	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
		set pyi $posy_dis
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$d_moduleret+2*$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}

	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret] $posy_dis\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$d_moduleret+2*$dimeny_rettang]
	}
   }
 }

 if {$num_rett == 3} {		;# disegno 3 rettangoli uguali

   if {$rett_reverse == 1} {	;# creo il rett.1 e lo riempio 

	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
		set pyi $posy_dis
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -F1 -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}

	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret] $posy_dis\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$dimeny_rettang]\
      		-fill black
	}

   } else {			;# creo il rett.1 vuoto

	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
		set pyi $posy_dis
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}

	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret] $posy_dis\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$dimeny_rettang]
	}
   }			

   if {$rett_reverse == 2} {	;# creo il rett.2 e lo riempio 

	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
      		set pyi [expr $posy_dis+$dimeny_rettang+$d_moduleret]
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$d_moduleret+2*$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -F1 -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}


	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$dimeny_rettang+$d_moduleret]\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$d_moduleret+2*$dimeny_rettang]\
      		-fill black

	}

   } else {			;# creo il rett.2 vuoto

	if { $tipovis==1 } {

      		set pxi [expr $posx_dis+$dimenx_rettang+$d_moduleret]
      		set pyi [expr $posy_dis+$dimeny_rettang+$d_moduleret]
      		set pxf [expr $posx_dis+2*$dimenx_rettang+$d_moduleret]
      		set pyf [expr $posy_dis+$d_moduleret+2*$dimeny_rettang]
		if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
		}


	} else {

      		$canv create rect\
      		[expr $posx_dis+$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$dimeny_rettang+$d_moduleret]\
      		[expr $posx_dis+2*$dimenx_rettang+$d_moduleret]\
      		[expr $posy_dis+$d_moduleret+2*$dimeny_rettang]

	}

   }
 }
}



######################################################################
#
# DESCR:MODULO - restituisce il testo di descrizione del modulo
#
######################################################################
proc descr:modulo {tipo} {

	global	indiceincorso
	global	desc_SOR88 desc_SO24 desc_SO816 desc_SO8
	global	desc_SIR88 desc_SI24 desc_SI816 desc_SI8
 
 if {$tipo =="SOR88"} {
   		return $desc_SOR88
 }
 if {$tipo =="SO24"} {
   		return $desc_SO24
 }
 if {$tipo =="SI8"} {
   		return $desc_SI8
 }
 if {$tipo =="SO816"} {
   		return $desc_SO816
 }
 if {$tipo =="SO8"} {
   		return $desc_SO8
 }
 if {$tipo =="SIR88"} {
   		return $desc_SIR88
 }
 if {$tipo =="SI24"} {
   		return $desc_SI24
 }
 if {$tipo =="SI816"} {
   		return $desc_SI816
 }
}

############################################################################
##
## CREA:BLOCCO
## -----------
##
## NOTE: disegna i blocchi che compongono un modulo
##       "num_block" = numero del blocco da disegnare
##       "tipo" = tipo del blocco ( input, static output, relay output).
##	 "num_primopiede" = numero del primo piedino del blocco (normalmente 0)
##
############################################################################

proc Crea:Blocco {canv posix posiy num_block tipo scheda num_primopiede} {

  global SpesMed indiceincorso dim_moduloy_mpp dim_moduloy_udc
  global dim_modulox dim_moduloy pixelalttitfp pixellartitfp
  global des_input text_byte distscrtesto tipovis font_grande
  global font_titfp font_titolbold outst outrele xf_pagina font_testi
  global Linea_in_Corso Modulo_in_Corso PuntiPerRiga punti_per_mm
  global VariableList font_titolbold MezzaRiga
  global dist_testilinee pixellargrande font_testibig

  if { $scheda=="mpp" } {
	set speciale 1
  	set dimy $dim_moduloy_mpp
	set num_partiz 25	;# 8 OUT + 16 INP + Intestazione
	set num_output 8
  } elseif { $scheda=="udc" } {
	set speciale 1
  	set dimy $dim_moduloy_udc
	set num_partiz 29	;# 12 OUT + 16 INP + Intestazione
	set num_output 12
  } else {
	set speciale 0
  	set dimy $dim_moduloy
	set num_partiz 9	;# 8 I/O + Intestazione
	set num_output 0	;# in questo caso non serve
  }

  set posfx [expr $posix+$dim_modulox]		;# posiz. finale x del modulo
  set posfy [expr $posiy+$dimy]			;# posiz. finale y del modulo
  set x_sep [expr $posfx-15*$punti_per_mm]	;# X separaz. nomevar - piedino
  set offset	[expr $dimy/$num_partiz]
  set ylinea	[expr $posiy+$offset]

#
# DISEGNA LA GRIGLIA PRINCIPALE DEL MODULO
# (rettangolo + linea separazione nome variabile / numero piedino)
#

  if { $tipovis==1 } {
	set pxi $posix
	set pyi $posiy
	set pxf $posfx
	set pyf [expr 2 + $posfy]
	if { $indiceincorso==1 } {
	PrtPoli -S$SpesMed -P$pxi,$pyi,$pxf,$pyi,$pxf,$pyf,$pxi,$pyf
	PrtLine -S$SpesMed -P$x_sep,$ylinea,$x_sep,$pyf
	}

  } else {
	$canv create rect $posix $posiy $posfx $posfy -width $SpesMed
  }

#
# TESTO PER IL TIPO DI MODULO NEL PRIMO RETTANGOLINO DELLO SCHEMA
#

  if { $tipovis==1 } {

  	set posy_testo	[expr $posiy+$dist_testilinee]
	set testap $scheda		;# default

	if {$tipo == "input"} {		;# tipo di modulo = 8 Input
		set testap $des_input
	}
	if {$tipo == "static_out"} {	;# tipo di modulo = 8 Output Statiche
		set testap $outst
	}
	if {$tipo == "rele_out"} {	;# tipo di modulo = 8 Output Relay
		set testap $outrele
	}
   	crea:testo:centrato:bold $posix $posfx $posy_testo $testap $font_testi 

  } else {

  	set posy_testo	[expr $posiy+$offset/2]
	set testap $scheda		;# default

	if {$tipo == "input"} {		;# tipo di modulo = 8 Input
			set testap $des_input
	}
	if {$tipo == "static_out"} {	;# tipo di modulo = 8 Output Statiche
			set testap $outst
	}
	if {$tipo == "rele_out"} {	;# tipo di modulo = 8 Output Relay
			set testap $outrele
	}

	set xi [posiz:testo:centrato $posix $posfx $testap $pixellartitfp]
   	crea:testo $canv $xi $posy_testo $testap $font_titfp 
  }

#
# LOOP PER IL DISEGNO DELLE PARTIZIONI DEL MODULO + NUMERAZIONE
#

  for {set i 0} {$i<[expr $num_partiz-1]} {incr i} {

	set Piedino_in_Corso $i
	set Blocco_in_Corso  $num_block

	#
	# TRACCIA LA LINEA DI SEPARAZIONE TRA I VARI "PIEDINI"
	#

   	if { $tipovis==1 } {

		set num_piede [expr $num_primopiede + $i]

		if { $indiceincorso==1 } {
			PrtLine -S$SpesMed -P$posix,$ylinea,$posfx,$ylinea
		}

   		if {$tipo == "input"} {

			set testap "I"

   		} elseif { $tipo == "inout" } {

			if { $i < $num_output } {
				set testap "O"
			} else {
				set testap "I"
				set num_piede [expr $i - $num_output]
			}
		} else {
			set testap "O"
		}
      		crea:testo:bold $canv [expr $x_sep+$distscrtesto]\
      			[expr $ylinea + $dist_testilinee] $testap $font_grande

   		crea:testo $canv [expr $x_sep+3*$punti_per_mm+$pixellargrande]\
   			[expr $ylinea + 2*$dist_testilinee]\
			$num_piede $font_titfp 

	} else {

   		$canv create line $posix $ylinea $posfx $ylinea

		set num_piede [expr $num_primopiede + $i]

   		if {$tipo == "input"} {

			set testap "I"

   		} elseif { $tipo == "inout" } {

			if { $i < $num_output } {
				set testap "O"
			} else {
				set testap "I"
				set num_piede [expr $i - $num_output]
			}

		} else {
				set testap "O"
   		}

      		crea:testo $canv [expr $x_sep+2*$punti_per_mm]\
      			[expr $ylinea+$offset/2] $testap $font_testibig 
   		crea:testo $canv [expr $x_sep+6*$punti_per_mm]\
   			[expr $ylinea+$offset/2] $num_piede $font_titfp 
	}

	#
	# LOOP DI RICERCA NOME VARIABILE ASSOCIATA AD UN INGRESSO/USCITA
	#

   	set testo  ""
	set koment ""

	foreach y [lsort -ascii [array names VariableList]] {

	    set tipomodvar [string toupper [lindex $VariableList($y) 8]]

	    if { $tipomodvar=="IOS" } {
		#
		# questa parte viene gestita solo se la Crea:Blocco e`
		# stata chiamata per i moduli NON SPECIALI
		#
		if { $speciale==0 } {
		    set piedino [lindex $VariableList($y) 11]

		    if { $piedino >= 0 && $piedino < 100 } {
			set app_blocco [expr ($piedino /8)%3 ]
			set app_piedino [expr $piedino %8 ]
		    } else {
			set app_blocco 9999
			set app_piedino 9999
		    }

		    set app_modulo [lindex $VariableList($y) 10]
		    set app_ioline [lindex $VariableList($y) 9]
    
		    if {$app_piedino==$Piedino_in_Corso	&&\
		        $app_blocco==$Blocco_in_Corso	&&\
		        $app_modulo==$Modulo_in_Corso	&&\
		        $app_ioline==$Linea_in_Corso	} {

			if { [lindex $VariableList($y) 3] == "input" } {
				if { $tipo == "input" } {
					set testo "$y"
					set koment [lindex $VariableList($y) 5]
				}
			}
			if { [lindex $VariableList($y) 3] == "output" } {
				if { $tipo == "static_out" || $tipo == "rele_out" } {
					set testo "$y"
					set koment [lindex $VariableList($y) 5]
				}
			}
		    }
		}
	    } else {
		#
		# questa parte viene gestita solo se la Crea:Blocco e`
		# stata chiamata per i moduli SPECIALI (mpp,udc....)
		#
		if { $speciale==1 } {
	          if { [string first "UDC" $tipomodvar] != -1 } {
		    if { $scheda=="udc" } {
			#
			# variabile in una UDC
			#
			set numinp 16
			set numout 12
			set tmod "udc"

    			if { [lindex $VariableList($y) 8] != {} && \
      				[lindex $VariableList($y) 1] == "BOOL" } {

        		    if { [string first "UDC" $tipomodvar] != -1} {

          			set ElementiConnessione [split \
					[lindex $VariableList($y) 8] "."]
				set linvar [lindex $ElementiConnessione 1]
          			set piedin [lindex $ElementiConnessione end]

          			if { [lindex $VariableList($y) 3] == "input"} {
				    set piedin [expr $piedin + $numout]
				}

				if { $linvar == $Linea_in_Corso && \
					$piedin == $Piedino_in_Corso } {
				    set inpoout [lindex $VariableList($y) 3]
				    set koment [lindex $VariableList($y) 5]
				    set testo $y
				}
			    }
			}
		    }
		} elseif { [string first "MPP" $tipomodvar] != -1 } {
		    if { $scheda=="mpp" } {
			#
			# variabile in una MPP
			#
			set numinp 16
			set numout 8
			set tmod "mpp"

    			if { [lindex $VariableList($y) 8] != {} && \
      				[lindex $VariableList($y) 1] == "BOOL" } {

        		    if { [string first "MPP" $tipomodvar] != -1} {

          			set ElementiConnessione [split \
					[lindex $VariableList($y) 8] "."]
				set linvar [lindex $ElementiConnessione 1]
          			set piedin [lindex $ElementiConnessione end]

          			if { [lindex $VariableList($y) 3] == "input"} {
				    set piedin [expr $piedin + $numout]
				}

				if { $linvar == $Linea_in_Corso && \
					$piedin == $Piedino_in_Corso } {
				    set inpoout [lindex $VariableList($y) 3]
				    set koment [lindex $VariableList($y) 5]
				    set testo $y
				}
			    }
			}
		     } ;# endif scheda
		} ;# endif string....
	      } ;# endif speciale
	    }
	}	;# FINE FOREACH VARIABILE

   	if { $tipovis==1 } {

   		crea:testo:centrato:bold $posix $x_sep\
   			[expr $ylinea + $dist_testilinee]\
			$testo $font_titolbold
   	} else {
   		set posx_testo [posiz:testo:centrato $posix $x_sep $testo\
   							$pixellartitfp]
   		crea:testo $canv $posx_testo\
   			[expr $ylinea+$offset/2-$pixelalttitfp/2]\
			$testo $font_titfp 
   	}

# commento di prova alla variabile

   	set testo $koment

   	if { $tipovis==1 } {

   		crea:testo:bold $canv [expr $posfx+2*$distscrtesto]\
   			[expr $ylinea + $dist_testilinee]\
			$testo $font_titolbold

	} else {

   		scrivi:testo $canv [expr $posfx+20]\
   			[expr $ylinea+$offset/2-$pixelalttitfp/2]\
			$testo $font_titfp\
   			[expr $xf_pagina-($posfx-20)]

	}
   	incr ylinea $offset
  }

# LINEA DI SEPARAZIONE NUMERO I/O E NOME DELLE VARIABILI E NUMERO DEL BYTE

  if { $speciale==0 } {
    if { $tipovis==1 } {

	# PrtLine -S$SpesMed -P$x_sep,[expr $posiy+$offset],$x_sep,$ylinea
  	crea:testo:centrato:bold $posix $posfx\
  		[expr $posiy - $PuntiPerRiga - $MezzaRiga ]\
		"$text_byte $num_block" $font_grande 
    } else {

	$canv create line $x_sep [expr $posiy+$offset] $x_sep $ylinea
  	set posx_testo [posiz:testo:centrato $posix\
			$posfx "$text_byte $num_block"\
  			$pixellartitfp]
  	crea:testo $canv\
  		$posx_testo [expr $posiy-$pixelalttitfp-2*$distscrtesto]\
		"$text_byte $num_block" $font_titfp 
    }
  }
}



###################################################################
##
## CREA:MODULO
## -----------
##
## NOTE: crea i blocchi che compongono ogni singolo modulo
##
###################################################################

proc Crea:Modulo {canv tipo_mod ini_scrx ini_scry } {

	global	indiceincorso
 	global 	dim_moduloy dim_modulox PuntiPerRiga y_titolofp
 	global 	xi_pagina xf_pagina tipovis punti_per_mm

 set dim_scrx [expr $xf_pagina-$xi_pagina]
 set distanza_blocchi [expr 3 * $PuntiPerRiga]

# POSIZIONI X E Y DEI BLOCCHI DA DISEGNARE PER IL MODULO

 set xprimo_blocco [expr $ini_scrx+($dim_scrx/2-$dim_modulox)/2]
 set xsecon_blocco $xprimo_blocco
 set xterzo_blocco $xsecon_blocco

 set yinizio [expr $ini_scry   + 2*$PuntiPerRiga] 
 set ymax    [expr $y_titolofp - 2*$PuntiPerRiga]
 set yfinale [expr $ymax + 1]

 set num_mm 50

 while { $yfinale > $ymax } {
   set yprimo_blocco $yinizio
   set ysecon_blocco [expr $yprimo_blocco+$dim_moduloy+$num_mm*$punti_per_mm]
   set yterzo_blocco [expr $ysecon_blocco+$dim_moduloy+$num_mm*$punti_per_mm]
   set yfinale [expr $yterzo_blocco+$dim_moduloy]
   incr num_mm -2
 }

# creo i blocchi che compongono ogni modulo

 if {$tipo_mod=="mpp"} {
   set tipo "inout"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "mpp" 0
 }

 if {$tipo_mod=="udc"} {
   set tipo "inout"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "udc" 0
 }

 if {$tipo_mod=="SOR88"} {
   set tipo "static_out"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   set tipo "rele_out"
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0

   set num_rett 2		;# disegno 2 rettangoli : 1 piccolo 1 grande
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 12 $num_rett
 }

 if {$tipo_mod=="SO24"} {
   set tipo "static_out"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 0

   set num_rett 3		;# disegno 3 rettangoli
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 1 $num_rett
   modulo:schematizz $canv $xterzo_blocco $yterzo_blocco 2 $num_rett
 }

 if {$tipo_mod=="S24T_I"} {
   set tipo "input"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 8
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 16
 }

 if {$tipo_mod=="S24T_O"} {
   set tipo "static_out"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 8
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 16
 }

 if {$tipo_mod=="SI8"} {
   set tipo "input"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0

   set num_rett 1		;# disegno 1 rettangolo
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
 }

 if {$tipo_mod=="SO816"} {
   set tipo "static_out"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   set tipo "input"
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 0

   set num_rett 3		;# disegno 3 rettangoli
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 1 $num_rett
   modulo:schematizz $canv $xterzo_blocco $yterzo_blocco 2 $num_rett
 }

 if {$tipo_mod=="SO8"} {
   set tipo "static_out"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0

   set num_rett 1		;# disegno 1 rettangolo
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
 }

 if {$tipo_mod=="SIR88"} {
   set tipo "input"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   set tipo "rele_out"
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0

   set num_rett 2		;# disegno 2 rettangoli : 1 piccolo 1 grande
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 12 $num_rett
 }

 if {$tipo_mod=="SI24"} {
   set tipo "input"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 0

   set num_rett 3		;# disegno 3 rettangoli
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 1 $num_rett
   modulo:schematizz $canv $xterzo_blocco $yterzo_blocco 2 $num_rett
 }

 if {$tipo_mod=="SI816"} {
   set tipo "input"
   Crea:Blocco $canv $xprimo_blocco $yprimo_blocco 0 $tipo "ios" 0
   set tipo "static_out"
   Crea:Blocco $canv $xsecon_blocco $ysecon_blocco 1 $tipo "ios" 0
   Crea:Blocco $canv $xterzo_blocco $yterzo_blocco 2 $tipo "ios" 0

   set num_rett 3		;# disegno 3 rettangoli
   modulo:schematizz $canv $xprimo_blocco $yprimo_blocco 0 $num_rett
   modulo:schematizz $canv $xsecon_blocco $ysecon_blocco 1 $num_rett
   modulo:schematizz $canv $xterzo_blocco $yterzo_blocco 2 $num_rett
 }
}


###########################################################################
##
## CREA:BLOCCO:RIEP:MODULI
##
## NOTE: crea il riepilogo delle linee e dei moduli che ne fanno parte
##
#############################################################################

proc crea:blocco:riep:moduli {canv xi yi xf yf num_line linea_moduli} {

	global	indiceincorso
	global 	SpesMed fnt3x4 MezzaRiga pixellartestobig
	global	distscrtesto line_n pixelalttesto modulo
	global	font_titolbold max_num_moduli tipovis elenco_moduli
	global 	pixelalttitfp pixellartitfp font_titfp num tipo_modulo moduli
	global 	font_testibig pixelalttestobig
	global 	punti_per_mm dist_testilinee PuntiPerRiga font_testi
	global  flag_riep

 set flag_riep 1

#
# CONTA MODULI PRESENTE NELLA LINEA
#

 if { $tipovis==1 } {
	set font_appo $fnt3x4
 } else {
	set font_appo $font_testi
 }

 set altprig	 [expr 18*$punti_per_mm]	;# altezza prima riga (titolo)

 set totale [expr $yf - $yi]			;# spazio totale su Y
 set app1 [expr $totale-$altprig-$PuntiPerRiga]	;# spazio su Y per la "griglia"
 set app2 [expr $app1%($max_num_moduli/2+1)]
 set altrig [expr $app1/($max_num_moduli/2+1)]	;# altezza della riga
 set lungh [conta:moduli $linea_moduli]

 if {$lungh>1} {
	set testo "$line_n $num_line : $lungh $moduli"
 } else {
	set testo "$line_n $num_line : $lungh $modulo"
 }

# intestazione del blocco di riepilogo

 if { $tipovis==1 } {
	set posy [expr $yi+$altprig-$PuntiPerRiga]
	crea:testo:centrato:bold $xi $xf $posy $testo $font_testibig
	set posy [expr $yi+$altprig]
	set appo [expr $posy+$altrig*($max_num_moduli/2+1)]
	set xic $xi
	set yic $posy
	set xfc $xf
	set yfc $appo

 } else {
	set posy [expr $yi+$altprig-$PuntiPerRiga]
	set appo [posiz:testo:centrato $xi $xf $testo $pixellartestobig]
	crea:testo $canv $appo $posy $testo $font_testibig
	set posy [expr $yi+$altprig]
	set appo [expr $posy+$altrig*($max_num_moduli/2+1)]
	set xic $xi
	set yic $posy
	set xfc $xf
	set yfc $appo
 }

# cornice di contorno blocco

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
		PrtPoli -S$SpesMed -P$xic,$yic,$xfc,$yic,$xfc,$yfc,$xic,$yfc
	}
 } else {
		$canv create rect $xic $yic $xfc $yfc -width $SpesMed
 }

#
# LINEA VERTICALE DI SEPARAZIONE BLOCCO
#

 set xmeta [expr $xic+($xfc-$xic)/2]

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xmeta,$posy,$xmeta,$yfc
	}
 } else {
	$canv create line $xmeta $posy $xmeta $yfc
 }

# set disty_righe [expr ($yfc-$posy)/($max_num_moduli/2+1)]
 set disty_righe $altrig

#
# SCRIVE TESTI "N." E "TIPO MODULO"
#	E TRACCIA LINEA VERTICALE DI SEPARAZIONE PARTE DESTRA
#

 if { $tipovis==1 } {

	set xlinea1 [expr $xic + 10*$punti_per_mm]
	set xlinea2 [expr $xmeta + 10*$punti_per_mm]

	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xlinea1,$posy,$xlinea1,$yfc
	}

	set y [expr $posy+$dist_testilinee]

	crea:testo:bold $canv [expr $xic+$distscrtesto] $y $num $font_appo

	crea:testo:centrato:bold $xlinea1 $xmeta $y $tipo_modulo $font_appo

 } else {
	set xlinea1\
		[expr $xic+[string length $num]*$pixellartitfp+2*$distscrtesto]
	set xlinea2 [expr $xmeta+\
		[string length $num]*$pixellartitfp+2*$distscrtesto]
	$canv create line $xlinea1 $posy $xlinea1 $yfc

	set y [expr $posy+$disty_righe/2]
	crea:testo $canv [expr $xic+$distscrtesto]\
						$y $num $font_titfp  ;# "N."

	set appo [posiz:testo:centrato $xlinea1\
					$xmeta $tipo_modulo $pixellartitfp]
	crea:testo $canv $appo $y $tipo_modulo $font_titfp	;# "TIPO MODULO"
 }


#
# SCRIVE TESTI "N." E "TIPO MODULO"
#	E TRACCIA LINEA VERTICALE DI SEPARAZIONE PARTE DESTRA
#

if { $tipovis==1 } {

	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xlinea2,$posy,$xlinea2,$yfc
	}

	crea:testo:bold $canv [expr $xmeta+$distscrtesto] $y $num $font_appo

	crea:testo:centrato:bold $xlinea2 $xfc $y $tipo_modulo $font_appo

	incr posy $disty_righe

} else {
	$canv create line $xlinea2 $posy $xlinea2 $yfc
	crea:testo $canv [expr $xmeta+$distscrtesto]\
						$y $num $font_titfp   ;# "N."

	set appo\
		[posiz:testo:centrato $xlinea2 $xfc $tipo_modulo $pixellartitfp]
	crea:testo $canv $appo $y $tipo_modulo $font_titfp  ;# "TIPO MODULO"

	incr posy $disty_righe
}


#
# LOOP PER DISEGNO LINEE ORIZZONTALI E SCRITTURA NUMERO DEL MODULO
#

 for {set i 0} {$i<[expr $max_num_moduli/2]} {incr i} {

# scrittura numero dei moduli

   if {$tipovis==1} {
   	set appo [posiz:testo:centrato $xic $xlinea1 $i $pixellartitfp]
   	crea:testo:centrato:bold $xic $xlinea1 [expr $posy+$dist_testilinee]\
   								$i $font_appo
   	crea:testo:centrato:bold $xmeta $xlinea2 [expr $posy+$dist_testilinee]\
   				[expr $i+$max_num_moduli/2] $font_appo
   } else {
   	set appo [posiz:testo:centrato $xic $xlinea1 $i $pixellartitfp]
   	crea:testo $canv $appo [expr $posy+$disty_righe/2] $i $font_titfp
   	set appo [posiz:testo:centrato $xmeta $xlinea2\
   				[expr $i+$max_num_moduli/2] $pixellartitfp]
   	crea:testo $canv $appo [expr $posy+$disty_righe/2]\
   				[expr $i+$max_num_moduli/2] $font_titfp
   }

# scrittura linea orizzontale di separazione

   if {$i == 0} {		;# prima linea : la disegno piu' grossa
      if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xic,$posy,$xfc,$posy
	}
      } else {
      	$canv create line $xic $posy $xfc $posy -width 2.0 
      }
   } else {
      if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xic,$posy,$xfc,$posy
	}
      } else {
      	$canv create line $xic $posy $xfc $posy
      }
   }
   incr posy $disty_righe
 }

# loop per la scrittura dei moduli

 set posy [expr $yic+$disty_righe]

 for {set i 0} {$i<$max_num_moduli} {incr i} {

  set tipo $elenco_moduli($num_line,$i)

  if { $tipo!=-1 } {

   if {$i<[expr $max_num_moduli/2]} {
      set xlinea $xlinea1
   } else {
      set xlinea $xlinea2
   }
   if {$i == [expr $max_num_moduli/2]} {		;# riscrivo in alto
      set posy [expr $yic+$disty_righe]
   }
   if {$tipo!= ""} {					;# modulo presente

	if { $tipovis==1 } {
      		crea:testo:bold $canv [expr $xlinea+2*$distscrtesto]\
      			[expr $posy+$dist_testilinee] $tipo $font_appo

      		set testo [descr:modulo $tipo]

      		crea:testo $canv [expr $xlinea+10*$pixellartitfp]\
      			[expr $posy+$dist_testilinee] $testo $font_appo

	} else {
      		crea:testo $canv [expr $xlinea+2*$distscrtesto]\
      			[expr $posy+$disty_righe/2] $tipo $font_titolbold
      		set testo [descr:modulo $tipo]		;# descrizione modulo
      		crea:testo $canv [expr $xlinea+10*$distscrtesto+$distscrtesto]\
      			[expr $posy+$disty_righe/2] $testo $font_titfp
	}
   }

  }
  incr posy $disty_righe
 }
}


##################################################
##						##
## PAGINA:RIEP:MODULI				##
## ------------------				##
##						##
## NOTE: crea la pagina di riepilogo moduli	##
##						##
##################################################

proc pagina:riep:moduli {canv num_pagina} {

	global	indiceincorso
	global	SpesMed pixellartestobig 
	global	cornice TipoTabIO dist_testilinee punti_per_mm
	global	pixelaltgrande pixellargrande font_grande distscrtesto 
	global	pixelaltenorme titolo_riep pixellarenorme font_testibig
	global	font_enorme tipovis xi_scrit xf_scrit font_granden
	global	dimfoglioy numline_riep y_titolofp frameModuli max_num_linee 
	global	dimfogliox xi_pagina xf_pagina yi_pagina yf_pagina
	global  SaltaFormFeed tipovis dx_pagina dy_pagina
	global	NumeroDellaPagina Nomfil_debug PuntiPerRiga

 set tipoStp $TipoTabIO		;# Titolo di fine pagina
 set filestampa $Nomfil_debug

 set conta_pag 0		;# numero per il file post-script

#
# CONTEGGIO DELLE LINEE DI I/O EFFETTIVAMENTE UTILIZZATE
#

 set linee_usate 0

 for {set i 0} {$i<$max_num_linee} {incr i} {

	set appo [conta:moduli $i ]

	if {$appo != 0} {
		incr linee_usate
		set arr_lin($i) 1
	} else {
		set arr_lin($i) 0
	}
 }

#
# LOOP PRODUZIONE PAGINA DI "RIEPILOGO MODULI"
#
  
 set conta_lin 0

 for {set i 0} {$i<$linee_usate} {incr i 2} {

# prima di ogni cosa, svuoto completamente il canvas

    if { $tipovis==1 } {
		Next_Page
    } else {
    	if [info exist frameModuli] {
       		$canv delete tagMod
    	}
    }

    if {$cornice} {
       crea:cornice $canv $xi_pagina $yi_pagina $xf_pagina $yf_pagina 
    }

# scrittura intestazione pagina

    if { $tipovis==1 } {
    	set dimensione_intestazioni [expr $punti_per_mm*9]
    } else {
    	set dimensione_intestazioni [expr $punti_per_mm*7]
    }

    set posy [expr $yi_pagina+$dimensione_intestazioni ]

    if { $tipovis==1 } {

	crea:testo:centrato:bold $xi_pagina $xf_pagina\
		[expr $yi_pagina+2*$dist_testilinee] $titolo_riep $font_enorme
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}

    } else {

   	$canv create line $xi_pagina $posy $xf_pagina $posy
	set xi [posiz:testo:centrato $xi_pagina $xf_pagina\
					$titolo_riep $pixellartestobig]
   	crea:testo:bold $canv $xi [expr $posy-$dimensione_intestazioni/2]\
					$titolo_riep $font_testibig
   	set posy [expr $posy+$dimensione_intestazioni]
   	$canv create line $xi_pagina $posy $xf_pagina $posy
    }

    set testo "$numline_riep $linee_usate"

    if { $tipovis==1 } {
   	crea:testo:centrato $xi_pagina $xf_pagina\
		[expr $posy+2*$dist_testilinee] $testo $font_grande

   	set posy [expr $posy + $dimensione_intestazioni]

    } else {
   	set testo "$numline_riep $linee_usate"
   	set xi [posiz:testo:centrato\
			$xi_pagina $xf_pagina $testo $pixellartestobig]
   	crea:testo $canv $xi [expr $posy-$dimensione_intestazioni/2]\
 						$testo $font_granden
    }

# LINEA DI SEPARAZIONE INTESTAZIONE

    if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}
    } else {
	$canv create line $xi_pagina $posy $xf_pagina $posy
    }

# SE NO CORNICE, CREA CONTORNO INTESTAZIONE

    if {$cornice=="false"} {
      	if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$yi_pagina,$xf_pagina,$yi_pagina
		PrtLine -S$SpesMed -P$xi_pagina,$yi_pagina,$xi_pagina,$posy
		PrtLine -S$SpesMed -P$xf_pagina,$yi_pagina,$xf_pagina,$posy
		}
      	} else {
		$canv create line $xi_pagina $yi_pagina $xf_pagina $yi_pagina
		$canv create line $xi_pagina $yi_pagina $xi_pagina $posy
		$canv create line $xf_pagina $yi_pagina $xf_pagina $posy
      	}
    }

# LOOP PER DISEGNO DEI BLOCCHI PER DESCRIZIONE LINEA 

    set ymeta [expr $posy+($y_titolofp-$posy)/2]
    set j 0

    while {($conta_lin<$max_num_linee) && ($j<2)} {

      if {$arr_lin($conta_lin) == 1} {		;# linea piena

         if {$j == 0} {
            crea:blocco:riep:moduli $canv $xi_scrit $posy\
			$xf_scrit [expr $ymeta-$PuntiPerRiga]\
							$conta_lin $conta_lin
         }
         if {$j == 1} {
            crea:blocco:riep:moduli $canv $xi_scrit $ymeta\
			$xf_scrit [expr $y_titolofp-$PuntiPerRiga]\
							$conta_lin $conta_lin
         }
         incr j
      }
      incr conta_lin
    }

    crea:titolo:finepag $canv $tipoStp
    incr conta_pag

    if { $tipovis!=1 } {
    	$canv addtag tagMod all

	if { $tipovis==3 } {
    		set nomefile "$filestampa$NumeroDellaPagina" 
		crea:postscript $frameModuli.canvMod $nomefile
	}
    }
 }

 set appo [expr $linee_usate/2]
 if {[expr $linee_usate%2] != 0} {
	incr appo
 }

 return $appo		;# numero di pagine scritte
}




##
##
## STAMPA:TESTO:PLC - crea canvas e pagine di stampa per programma PLC
## -------------------------------------------------------------------
##

proc stampa:testo:plc { t } {

	global	indiceincorso
 	global	OK_INITIALIZE dist_testilinee PuntiPerRiga NumeroDellaPagina
	global	y_titolofp distscrtesto cornice pixelalttesto dimfogliox
	global	dimfoglioy TipoPrgIL xi_scrit yi_scrit MezzaRiga
	global	NumPagPLC xi_pagina xf_pagina yi_pagina yf_pagina 
	global	pixellargrande font_grande tx_note pixelalttitfp
	global	font_titfp font_testi tipovis SaltaFormFeed
	global	SpesMed nomefileplc nomefilesenzaplc
	global	Nomfil_debug font_granden pixelalttestobig font_pikkolo

 set nomedelfileconpath [GetFilename $t]
 if { [file extension $nomedelfileconpath] == "" } {
   set nomedelfileconpath $nomedelfileconpath.plc
 }

 # apre file da stampare
 if [catch { set filest [open "$nomedelfileconpath" r]  } result] {
    AlertBox -text $result
    return
  }
 set numpage 0

 set filestampa $Nomfil_debug

 coord:pagina $dimfogliox $dimfoglioy
 posiz:titolo:finepag		;# calcolo la posizione del titolo di fine pag.

# calcolo le righe per ogni pagina ed il numero di pagine

 set spazio_scrit [expr $y_titolofp-$yi_scrit-$distscrtesto]

 set righeperpag 99999

 if { $tipovis==1 } {
	set righeperpag 0
	set altragg 0
	while { $altragg < $spazio_scrit } {
		incr righeperpag
		set altragg [expr $altragg + $pixelalttesto + 1 ]
	}
 	incr righeperpag -4
 }

 if { $tipovis==3 } {
	set righeperpag 0
	set altragg 0
	while { $altragg < $spazio_scrit } {
		incr righeperpag
		set altragg [expr $altragg + 2*$pixelalttesto + 1 ]
	}
 	incr righeperpag -4
 }

 set contarighe 0	;# contatore righe totali da stampare
 set righenot	0	;# righe da non stampare: fuori dal testo programma
 set righenot2	0	;# righe da non stampare: variabili "locali"
 set numpagine $numpage
 set inizio_trovato 0
 set gestite_locali 0	;# flag gestione variabili locali avvenuta
 set trovate_locali 0	;# flag presenza variabili locali
 set saltate_locali 0	;# flag saltate variabili locali

 while {![eof $filest]} {	;# loop per il conteggio pagine

   gets $filest strst

   if { $inizio_trovato==0 } {
  	set primo [ lindex $strst 0 ]

	if { $primo=="PROGRAM" } {
		set inizio_trovato 1
		incr contarighe
	} else {
		#
		# inizio programma non ancora trovato
		# incremento numero righe da non stampare
		#
		incr righenot
	}
   } else {
	#
	# sono all'interno del programma: se trovo le istruzioni "VAR" ed
	# "END_VAR" non stampo niente di quello che c'e` in mezzo
	#
	if { $gestite_locali==0 } {
		#
		# non ancora trovate variabili locali: controlla se trova
		# istruzione "VAR" o "END_VAR"
		#
  		set primo [ lindex $strst 0 ]
		if { $trovate_locali==0 } {
			#
			# cerco istruzione "VAR"
			#
			if { $primo=="VAR" } {
				set trovate_locali 1
				incr righenot2
				incr righenot2
				incr contarighe	-1
			} else {
				incr contarighe
			}
		} else {
			#
			# cerco istruzione "END_VAR"
			#
			incr righenot2
			if { $primo=="END_VAR" } {
				set gestite_locali 1
			}
		}
	} else {		;# else di if...gestite_locali....
		incr contarighe
	}
   }

# controllo se ho raggiunto il numero righe per pagina o se ho dei "^L"

   set ctrl_l "\f"

   if {($contarighe==$righeperpag) || ([string first $ctrl_l $strst] != -1)} {
      set contarighe 0			;# ricomincio con il conteggio
      incr numpagine			;# incremento il numero delle pagine
   }
 }

 incr contarighe -1			;# ne conta una in piu'

 if {($contarighe >0) && ($contarighe < $righeperpag)} { ;# pagina non piena
   incr numpagine
 }

 set xi_scrit [expr $xi_scrit+5*$distscrtesto]

 seek $filest 0 start 			;# mi riposiziono ad inizio file

# salta tutte le righe da non stampare

 for { set i 0 } { $i < $righenot } { incr i } {
      gets $filest strst
 }

# creazione della toplevel con titolo e del canvas

 if { $tipovis==1 } {
	set CanvPro "ProvaCNI"
 } else {
 	wm withdraw .
 	toplevel .topPrgPLC
 	wm title .topPrgPLC "STAMPA PROGRAMMA PLC"
        wm protocol .topPrgPLC WM_DELETE_WINDOW procx
 	set framePrgPLC [frame .topPrgPLC.frmplc]    ;# conterra' il canvas
 	set CanvPro [canvas $framePrgPLC.canvPLC\
			-width $dimfogliox -height $dimfoglioy]
 }

# loop per il disegno di tutte le pagine

 set nriga 0

 for {set i 0} {$i<$numpagine} {incr i} {

   if { $tipovis==1 } {
 		Next_Page
   } else {
   	if [info exist framePrgPLC] {
      		$CanvPro delete tagPLC
   	}
   }

   if {$cornice} {
      crea:cornice $CanvPro\
      $xi_pagina $yi_pagina $xf_pagina $yf_pagina 
   }

# SCRITTURA DEL NOME PROGRAMMA + LINEA DI CONTORNO. 

# controllo la lunghezza del nome del programma : se e' piu' lunga dello
# spazio di scrittura , la tronco.

   if { $tipovis==1 } {
   	set posy [expr $yi_pagina+2*$dist_testilinee]
   } else {
   	set posy [expr $yi_pagina+$PuntiPerRiga]
   }

   set xi [posiz:testo:centrato $xi_pagina $xf_pagina $nomefilesenzaplc\
          $pixellargrande]

   crea:testo:bold $CanvPro $xi $posy $nomefilesenzaplc $font_grande

   set posy [expr $yi_pagina+2*$PuntiPerRiga]

   if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}
   } else {
   	$CanvPro create line $xi_pagina $posy $xf_pagina $posy
   }

# scrittura testo per le note + linea di contorno

   if { $tipovis==1 } {
   	set testo "$tx_note"
   	crea:testo $CanvPro $xi_scrit [expr $dist_testilinee+$posy]\
							$testo $font_titfp
	set posy [expr $posy+$PuntiPerRiga]
   } else {
   	set testo "$tx_note"
	set posy [expr $posy+$PuntiPerRiga]
   	crea:testo $CanvPro $xi_scrit [expr $posy-$MezzaRiga]\
							$testo $font_titfp
   }


   if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}
   } else {
	$CanvPro create line $xi_pagina $posy $xf_pagina $posy
   }

# non ho la cornice , creo le linee di contorno

   if {$cornice == "false" } {		
	if { $tipovis==1 } {
		set xip $xi_pagina
		set xfp $xf_pagina
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xip,$yi_pagina,$xfp,$yi_pagina
		PrtLine -S$SpesMed -P$xip,$yi_pagina,$xip,$posy
		PrtLine -S$SpesMed -P$xfp,$yi_pagina,$xfp,$posy
		}
	} else {
		$CanvPro create line\
      			$xi_pagina $yi_pagina $xf_pagina $yi_pagina
		$CanvPro create line\
      			$xi_pagina $yi_pagina $xi_pagina $posy
		$CanvPro create line\
      			$xf_pagina $yi_pagina $xf_pagina $posy
	}
   }

# inizio dei testi dopo la linea di "NOTE"

   set yiniziale [expr $posy+4*$dist_testilinee]

   set j 0 
   while {($j<$righeperpag) && (![eof $filest])} {	;# loop scrittura testi

      gets $filest strst		;# leggo riga per riga

      if { $trovate_locali==1 && $saltate_locali==0 } {
	#
	# controlla se occorre "saltare" le righe del file
	# contenenti le "variabili locali"
	#
	set primo [ lindex $strst 0 ]		;# primo = istruzione o label
	if { $primo!="PROGRAM" } {
		set j_locali 0
		while { ($j_locali<$righenot2) } {
			#
			# loop salto variabili locali
			#
      			gets $filest strst	;# salta riga
			incr j_locali
		}
		set saltate_locali 1
	}
      }

      if { $tipovis==1 } {
      	set testy [expr $yiniziale+$j*$pixelalttesto]
      } else {
	if { $tipovis==3 } {
      		set testy [expr $yiniziale+2*$j*$pixelalttesto]
	} else {
      		set testy [expr $yiniziale+$j*$pixelalttestobig]
	}
      }

	set ctrl_l "\f"

      if {([string first $ctrl_l $strst] != -1)} {
	 set testovuoto " "
	 if { $tipovis==1 } {

		incr nriga
         	crea:testo $CanvPro $xi_scrit $testy\
					"$nriga	$testovuoto" $font_testi

	 } else {

		incr nriga
		set tabby "	"
         	scrivi:testo $CanvPro $xi_scrit $testy\
				"$nriga$testovuoto" $font_pikkolo\
						[expr $xf_pagina-$xi_pagina]
	 }
         set j $righeperpag	;# pagina piena : nuova pagina

      } else {

	 if { $tipovis==1 } {

		incr nriga
		set tabby "\t"
		set strst "$nriga\t$strst"

		#
		# sostituisce i TAB con i BLANK
		#
 		set tab "\t"
 		set spazio " "
 		set numcar 0
 		set numcarnontab 0
 		set strappo ""
 		set finecipol 0
 		while { $finecipol==0 } {

		 set aaa [string index $strst $numcar]
		 if { $aaa=="" } {
			set finecipol 1
		 } else {
			if { $aaa==$tabby } {
				#
				# via di carattere tab
				# calcola quanti spazi
				# occorre inserire
				#
				set lun $numcarnontab
               			set asso [expr $lun%8]
               			set numspazi [expr 8-$asso]
       				while { $numspazi!=0 } {
                       			set strappo "$strappo$spazio"
                       			incr numspazi -1
               			}
				set numcarnontab 0
				incr numcar
			} else {
				set strappo "$strappo$aaa"
				incr numcarnontab
				incr numcar
			}
		 }
 		}

		set strst $strappo
         	crea:testo $CanvPro $xi_scrit $testy\
						"$strst" $font_testi

	 } else {

		incr nriga
		set tabby "\t"
      		if { $tipovis==3 } {

			set strst "$nriga\t$strst"

			#
			# sostituisce i TAB con i BLANK
			#
 			set tab "\t"
 			set spazio " "
 			set numcar 0
 			set numcarnontab 0
 			set strappo ""
 			set finecipol 0
 			while { $finecipol==0 } {

			 set aaa [string index $strst $numcar]
			 if { $aaa=="" } {
				set finecipol 1
			 } else {
				if { $aaa==$tabby } {
					#
					# via di carattere tab
					# calcola quanti spazi
					# occorre inserire
					#
					set lun $numcarnontab
               				set asso [expr $lun%8]
               				set numspazi [expr 8-$asso]
       					while { $numspazi!=0 } {
                       				set strappo "$strappo$spazio"
                       				incr numspazi -1
               				}
					set numcarnontab 0
					incr numcar
				} else {
					set strappo "$strappo$aaa"
					incr numcarnontab
					incr numcar
				}
			 }
 			}

			set strst $strappo
         		scrivi:testo $CanvPro $xi_scrit $testy\
				"$strst" $font_pikkolo\
				[expr $xf_pagina-$xi_pagina]
      		} else {
         		scrivi:testo $CanvPro $xi_scrit $testy\
				"$nriga$tabby$tabby$strst" $font_pikkolo\
				[expr $xf_pagina-$xi_pagina]
		}

	 }
         incr j
      }
   }   

# titolo di fine pagina

   set tipoStp $TipoPrgIL			;# tipo provvisorio
   crea:titolo:finepag $CanvPro $tipoStp

   if { $tipovis!=1 } {

   	$CanvPro addtag tagPLC all

	if { $tipovis==3 } {

		set numpa [expr $i + 1]
		set nomefile "$filestampa$NumeroDellaPagina"
		crea:postscript $framePrgPLC.canvPLC $nomefile
	}
   }
 }

 if { $tipovis!=1 && $tipovis!=3} {
	pack $framePrgPLC.canvPLC -side left -fill both -expand true
	pack $framePrgPLC -side left -fill both -expand true
 }

 if { $tipovis==3 } {
	if [winfo exists .topPrgPLC] {
		destroy .topPrgPLC
 	}
 }

 close $filest

 set NumPagPLC $numpagine	;# memorizzo il numero pagine del PLC
}



##
##
## STAMPA:MODULI - crea canvas e pagine di stampa "moduli di I/O"
## --------------------------------------------------------------
##

proc stampa:moduli { } {

	global	indiceincorso
	global	SpesMed OK_INITIALIZE Nomfil_debug
 	global	dimfogliox dimfoglioy dist_testilinee
	global	distscrtesto pixellarenorme elenco_moduli PuntiPerRiga
 	global	cornice TipoTabIO frameModuli font_testi
 	global	pixelalttesto pixellartesto line_n xi_scrit tipovis
 	global	pixelalttitfp modulo_n pixellargrande font_grande font_titfp
 	global	tipo_modulo pixelaltenorme font_enorme tx_note
 	global	y_titolofp NumPagMSD xi_pagina xf_pagina yi_pagina yf_pagina
 	global	punti_per_mm max_num_linee max_num_moduli MezzaRiga
 	global	VariableList Linea_in_Corso Modulo_in_Corso SaltaFormFeed
 	global	Pagina_Riep_Moduli NumeroDellaPagina Pagina_Moduli 
	global	udc_usata mpp_usata linea_udc linea_mpp
	global	schede_speciali numero_schede_speciali
	global	num_parti_s24t s24tapp

 set numpage 1

 coord:pagina $dimfogliox $dimfoglioy
 posiz:titolo:finepag

 set tipoStp $TipoTabIO
 set filestampa $Nomfil_debug

 # creazione della toplevel con titolo e creazione del canvas

 if { $tipovis!=1 } {
 	wm withdraw .
 	toplevel .topModuli
 	wm title .topModuli "STAMPA MODULI"
        wm protocol .topModuli WM_DELETE_WINDOW procx
 	set frameModuli [frame .topModuli.frmMod]     ;# conterra' il canvas
 	set canvMods [canvas $frameModuli.canvMod\
			-width $dimfogliox -height $dimfoglioy]
 } else {
	set canvMods "ProvaCNI"
 }

 set numpagine $numpage			;# setta il contatore delle pagine

 set Pagina_Riep_Moduli $NumeroDellaPagina

 incr numpagine [pagina:riep:moduli $canvMods $Pagina_Riep_Moduli]

 set Pagina_Moduli $NumeroDellaPagina

#
# DISEGNA LA RAPPRESENTAZIONE DEI MODULI "IOS" COLLEGATI
#

#
# loop sulle linee
#
 for {set j 0} {$j<$max_num_linee} {incr j} {
   #
   # loop sui moduli di ogni linea
   #
   for {set i 0} {$i<$max_num_moduli} {incr i} {

      set Linea_in_Corso	$j
      set Modulo_in_Corso	$i

      #if [info exist frameModuli] {
      #		$canvMods delete tagMod
      #}
      
      
      if {[info exist elenco_moduli($j,$i)] == 0 } {
        continue
      }

      if {$elenco_moduli($j,$i) != -1} {

	#
	# MODULO PRESENTE:
	#
      	# TIPO MODULO=	[lindex $elenco_moduli($j,$i) 0]
      	# N. INPUT=	[lindex $elenco_moduli($j,$i) 1]
      	# N. OUTPUT=	[lindex $elenco_moduli($j,$i) 2]
      	# "IOS"=	[lindex $elenco_moduli($j,$i) 3]
	#

      	set tipo	[lindex $elenco_moduli($j,$i) 0]

	#
	# CONTROLLA SE IL MODULO E` TRA QUELLI NON-SPECIALI CHE
	# SONO TUTTAVIA COMPOSTI DA PIU` DI 24 I/O
	#
	set pagine_modulo 1		;# default: 1 pagina x modulo (GALLO)
	set tipo_app "normale"	;# GALLO

	if { $tipo == "S24T" } {			;# GALLO
		set pagine_modulo $num_parti_s24t	;# GALLO
		set tipo_app $tipo			;# GALLO
	}						;# GALLO

	for { set ppag 0 } { $ppag < $pagine_modulo } { incr ppag } {
	 #
	 # stampa pagina con  singola parte del modulo
	 #
         if [info exist frameModuli] {
         	$canvMods delete tagMod
         }

 	 if { $tipovis==1 } {
 		Next_Page
	 }

         if {$cornice} {
            crea:cornice $canvMods\
            		$xi_pagina $yi_pagina $xf_pagina $yf_pagina 
         }

       	 set yprima_lin $yi_pagina
       	 set ysec_lin [expr $yprima_lin+$PuntiPerRiga+$MezzaRiga ]
       	 set yterza_lin [expr $ysec_lin+2*$PuntiPerRiga+$MezzaRiga ]
       	 set yquarta_lin [expr $yterza_lin+$PuntiPerRiga+$dist_testilinee ]
 
# linee orizzontali per il contorno della intestazione pagina e per le note

	 if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed\
			-P$xi_pagina,$ysec_lin,$xf_pagina,$ysec_lin
		PrtLine -S$SpesMed\
			-P$xi_pagina,$yterza_lin,$xf_pagina,$yterza_lin
		PrtLine -S$SpesMed\
			-P$xi_pagina,$yquarta_lin,$xf_pagina,$yquarta_lin
		}
	 } else {
		$canvMods create line\
         		$xi_pagina $ysec_lin $xf_pagina $ysec_lin
		$canvMods create line\
         		$xi_pagina $yterza_lin $xf_pagina $yterza_lin
		$canvMods create line\
         		$xi_pagina $yquarta_lin $xf_pagina $yquarta_lin
	 }

# se non ho la cornice, creo le linee di contorno

         if {$cornice=="false"} {
		if { $tipovis==1 } {
			set xip $xi_pagina
			set xfp $xf_pagina
			if { $indiceincorso==1 } {
			PrtLine -S$SpesMed -P$xip,$yprima_lin,$xfp,$yprima_lin
			PrtLine -S$SpesMed -P$xip,$yprima_lin,$xip,$yquarta_lin
			PrtLine -S$SpesMed -P$xfp,$yprima_lin,$xfp,$yquarta_lin
			}
		} else {
			$canvMods create line\
				$xi_pagina $yprima_lin $xf_pagina $yprima_lin
			$canvMods create line\
				$xi_pagina $yprima_lin $xi_pagina $yquarta_lin
			$canvMods create line\
				$xf_pagina $yprima_lin $xf_pagina $yquarta_lin
		}
         }

# LINEA VERTICALE DI SEPARAZIONE MODULO/NUMERO LINEA

	 set spazio_mod	[expr 20*$distscrtesto]
	 set spazio_iol	[expr 20*$distscrtesto]

	 if { $tipovis==1 } {

         	set xlinea [expr $xi_pagina + $spazio_mod]
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xlinea,$yprima_lin,$xlinea,$yterza_lin
		}

	 } else {

         	set xlinea [expr $xi_pagina+5*$distscrtesto+\
         			[string length $modulo_n]*$pixellartesto]
		$canvMods create line $xlinea $yprima_lin $xlinea $yterza_lin

	 }

# SCRITTURA DEL TESTO "MODULO N." E NUMERO DEL MODULO

	 if { $tipovis==1 } {
         	crea:testo:centrato $xi_pagina $xlinea\
         		[expr $yprima_lin+$dist_testilinee]\
							$modulo_n $font_testi
         	crea:testo:centrato:bold $xi_pagina $xlinea\
			[expr $ysec_lin+$dist_testilinee]\
							$i $font_grande
	 } else {

         	crea:testo $canvMods [expr $xi_pagina+3*$distscrtesto]\
         		[expr $yprima_lin+$pixelalttesto] $modulo_n $font_testi
         	set xi [posiz:testo:centrato\
					$xi_pagina $xlinea $i $pixellargrande]
         	crea:testo $canvMods $xi\
			[expr $ysec_lin+($yterza_lin-$ysec_lin)/2]\
								$i $font_grande
	 }

# LINEA VERTICALE DI SEPARAZIONE LINEA/TIPO DI MODULO

	 if { $tipovis==1 } {
         	set xlinea2 [expr $xlinea + $spazio_iol]
		if { $indiceincorso==1 } {
         	PrtLine -S$SpesMed -P$xlinea2,$yprima_lin,$xlinea2,$yterza_lin
		}
	 } else {
         	set xlinea2 [expr $xlinea+5*$distscrtesto+\
         			[string length $line_n]*$pixellartesto]
         	$canvMods create line\
         			$xlinea2 $yprima_lin $xlinea2 $yterza_lin
	 }
   
# SCRITTURA DEL TESTO "I/O LINE" E NUMERO LINEA

	 if { $tipovis==1 } {
         	crea:testo:centrato $xlinea $xlinea2\
         		[expr $yprima_lin+$dist_testilinee] $line_n $font_testi
         	crea:testo:centrato:bold $xlinea $xlinea2\
			[expr $ysec_lin+$dist_testilinee] $j $font_grande
	 } else {
         	crea:testo $canvMods [expr $xlinea+3*$distscrtesto]\
         	[expr $yprima_lin+$pixelalttesto] $line_n $font_testi
         	set xi [posiz:testo:centrato $xlinea $xlinea2 $j $pixellargrande]
         	crea:testo $canvMods\
         	$xi [expr $ysec_lin+($yterza_lin-$ysec_lin)/2] $j $font_grande
	 }

# TESTO PER IL TIPO DI MODULO

	 if { $tipovis==1 } {

         	crea:testo:centrato $xlinea2 $xf_pagina\
         		[expr $yprima_lin+$dist_testilinee]\
			$tipo_modulo $font_testi
	
         	crea:testo:centrato:bold $xlinea2 $xf_pagina\
         		[expr $ysec_lin+$dist_testilinee] $tipo $font_enorme 
	
         	set testo [descr:modulo $tipo]	;# descrizione del modulo
	
         	crea:testo:centrato $xlinea2 $xf_pagina\
         		[expr $ysec_lin+$dist_testilinee+$PuntiPerRiga]\
         						$testo $font_testi 

	 } else {

         	set xi [posiz:testo:centrato\
			$xlinea2 $xf_pagina $tipo_modulo $pixellartesto]

         	crea:testo $canvMods $xi [expr $yprima_lin+$pixelalttesto]\
						$tipo_modulo $font_testi

         	set xi [posiz:testo:centrato\
				$xlinea2 $xf_pagina $tipo $pixellarenorme]
         	crea:testo $canvMods\
         		$xi [expr $ysec_lin+2*$MezzaRiga] $tipo $font_enorme 
	
         	set testo [descr:modulo $tipo]	;# descrizione del modulo
	
         	set xi [posiz:testo:centrato\
				$xlinea2 $xf_pagina $testo $pixellartesto]
         	crea:testo $canvMods\
         		$xi [expr $ysec_lin+4*$MezzaRiga] $testo $font_testi 
	 }

# TESTI PER LE NOTE 

         set testo "$tx_note"

	 if { $tipovis==1 } {
         	crea:testo $canvMods\
         		[expr $xi_scrit+$distscrtesto]\
         		[expr $yterza_lin+$dist_testilinee]\
         		$testo $font_titfp
	 } else {
         	crea:testo $canvMods\
         		[expr $xi_scrit+$distscrtesto]\
         		[expr $yterza_lin+$distscrtesto+$pixelalttitfp/2]\
         		$testo $font_titfp
	 }
   
# DISEGNO DEI BLOCCHI CHE COMPONGONO IL MODULO

	 if { $tipo_app == "normale" } {
         	Crea:Modulo $canvMods $tipo $xi_pagina $yquarta_lin
	 } else {
		if { $tipo_app == "S24T" } {
			Crea:Modulo $canvMods $s24tapp($ppag)\
				$xi_pagina $yquarta_lin
		}
	 }

# titolo di fine pagina

         crea:titolo:finepag $canvMods $tipoStp

         incr numpagine			;# conto le pagine 

# creazione del file postscript per ogni modulo

         if { $tipovis!=1 } {
		#
		#  per ora la produzione del post-script solo
		#  quando premo VISUALIZZA
		#
         	$canvMods addtag tagMod all

		if { $tipovis==3 } {
         		set nomefile "$filestampa$NumeroDellaPagina" 
			crea:postscript $canvMods $nomefile
		}
	 }
       }		;# end for sul numero pagine per il modulo (GALLO)
      }			;# end if se modulo presente
   }
 }

 #
 # Test se ci sono delle schede "speciali" come la MPP3, la UDC, ecc....
 #
 for { set scheda 0 } { $scheda < $numero_schede_speciali } { incr scheda } {

   for {set j 0} { $j<4 } {incr j} {

      set sche_app $schede_speciali($scheda)

      set Linea_in_Corso	$j
      set i 0

      set linlinea 0
      if {$sche_app=="mpp"} {
	set linlinea $linea_mpp($j)
      }
      if {$sche_app=="udc"} {
	set linlinea $linea_udc($j)
      }

      if {$linlinea != 0} {

         if [info exist frameModuli] {
         	$canvMods delete tagMod
         }

 	 if { $tipovis==1 } {
		Next_Page
	 }

	 #
	 # - SCHEDA SPECIALE PRESENTE ED UTILIZZATA
	 #

      	 set tipo	$sche_app

         if {$cornice} {
            crea:cornice $canvMods\
            		$xi_pagina $yi_pagina $xf_pagina $yf_pagina 
         }

       	set yprima_lin $yi_pagina
       	set ysec_lin [expr $yprima_lin+$PuntiPerRiga+$MezzaRiga ]
       	set yterza_lin [expr $ysec_lin+2*$PuntiPerRiga+$MezzaRiga ]
       	set yquarta_lin [expr $yterza_lin+$PuntiPerRiga+$dist_testilinee ]
 
# linee orizzontali per il contorno della intestazione pagina e per le note

	 if { $tipovis==1 } {
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed\
			-P$xi_pagina,$ysec_lin,$xf_pagina,$ysec_lin
		PrtLine -S$SpesMed\
			-P$xi_pagina,$yterza_lin,$xf_pagina,$yterza_lin
		PrtLine -S$SpesMed\
			-P$xi_pagina,$yquarta_lin,$xf_pagina,$yquarta_lin
		}
	 } else {
		$canvMods create line\
         		$xi_pagina $ysec_lin $xf_pagina $ysec_lin
		$canvMods create line\
         		$xi_pagina $yterza_lin $xf_pagina $yterza_lin
		$canvMods create line\
         		$xi_pagina $yquarta_lin $xf_pagina $yquarta_lin
	 }

# se non ho la cornice, creo le linee di contorno

         if {$cornice=="false"} {
		if { $tipovis==1 } {
			set xip $xi_pagina
			set xfp $xf_pagina
			if { $indiceincorso==1 } {
			PrtLine -S$SpesMed -P$xip,$yprima_lin,$xfp,$yprima_lin
			PrtLine -S$SpesMed -P$xip,$yprima_lin,$xip,$yquarta_lin
			PrtLine -S$SpesMed -P$xfp,$yprima_lin,$xfp,$yquarta_lin
			}
		} else {
			$canvMods create line\
				$xi_pagina $yprima_lin $xf_pagina $yprima_lin
			$canvMods create line\
				$xi_pagina $yprima_lin $xi_pagina $yquarta_lin
			$canvMods create line\
				$xf_pagina $yprima_lin $xf_pagina $yquarta_lin
		}
         }

# LINEA VERTICALE DI SEPARAZIONE MODULO/NUMERO LINEA

	 set spazio_mod	[expr 20*$distscrtesto]
	 set spazio_iol	[expr 20*$distscrtesto]

	 if { $tipovis==1 } {

         	set xlinea [expr $xi_pagina + $spazio_mod]
		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xlinea,$yprima_lin,$xlinea,$yterza_lin
		}

	 } else {

         	set xlinea [expr $xi_pagina+5*$distscrtesto+\
         			[string length $modulo_n]*$pixellartesto]
		$canvMods create line $xlinea $yprima_lin $xlinea $yterza_lin

	 }

# SCRITTURA DEL TESTO "MODULO N." E NUMERO DEL MODULO

	 if { $tipovis==1 } {
         	crea:testo:centrato $xi_pagina $xlinea\
         		[expr $yprima_lin+$dist_testilinee]\
							$modulo_n $font_testi
         	crea:testo:centrato:bold $xi_pagina $xlinea\
			[expr $ysec_lin+$dist_testilinee]\
							$i $font_grande
	 } else {

         	crea:testo $canvMods [expr $xi_pagina+3*$distscrtesto]\
         		[expr $yprima_lin+$pixelalttesto] $modulo_n $font_testi
         	set xi [posiz:testo:centrato\
					$xi_pagina $xlinea $i $pixellargrande]
         	crea:testo $canvMods $xi\
			[expr $ysec_lin+($yterza_lin-$ysec_lin)/2]\
								$i $font_grande
	 }

# LINEA VERTICALE DI SEPARAZIONE LINEA/TIPO DI MODULO

	 if { $tipovis==1 } {
         	set xlinea2 [expr $xlinea + $spazio_iol]
		if { $indiceincorso==1 } {
         	PrtLine -S$SpesMed -P$xlinea2,$yprima_lin,$xlinea2,$yterza_lin
		}
	 } else {
         	set xlinea2 [expr $xlinea+5*$distscrtesto+\
         			[string length $line_n]*$pixellartesto]
         	$canvMods create line\
         			$xlinea2 $yprima_lin $xlinea2 $yterza_lin
	 }
   
# SCRITTURA DEL TESTO "I/O LINE" E NUMERO LINEA

	 if { $tipovis==1 } {
         	crea:testo:centrato $xlinea $xlinea2\
         		[expr $yprima_lin+$dist_testilinee] $line_n $font_testi
         	crea:testo:centrato:bold $xlinea $xlinea2\
			[expr $ysec_lin+$dist_testilinee] $j $font_grande
	 } else {
         	crea:testo $canvMods [expr $xlinea+3*$distscrtesto]\
         	[expr $yprima_lin+$pixelalttesto] $line_n $font_testi
         	set xi [posiz:testo:centrato $xlinea $xlinea2 $j $pixellargrande]
         	crea:testo $canvMods\
         	$xi [expr $ysec_lin+($yterza_lin-$ysec_lin)/2] $j $font_grande
	 }

# TESTO PER IL TIPO DI MODULO

	 if { $tipovis==1 } {

         	crea:testo:centrato $xlinea2 $xf_pagina\
         		[expr $yprima_lin+$dist_testilinee]\
			$tipo_modulo $font_testi
	
         	crea:testo:centrato:bold $xlinea2 $xf_pagina\
         		[expr $ysec_lin+$dist_testilinee] $tipo $font_enorme 
	
         	set testo [descr:modulo $tipo]	;# descrizione del modulo
	
         	crea:testo:centrato $xlinea2 $xf_pagina\
         		[expr $ysec_lin+$dist_testilinee+$PuntiPerRiga]\
         						$testo $font_testi 

	 } else {

         	set xi [posiz:testo:centrato\
			$xlinea2 $xf_pagina $tipo_modulo $pixellartesto]

         	crea:testo $canvMods $xi [expr $yprima_lin+$pixelalttesto]\
						$tipo_modulo $font_testi

         	set xi [posiz:testo:centrato\
				$xlinea2 $xf_pagina $tipo $pixellarenorme]
         	crea:testo $canvMods\
         		$xi [expr $ysec_lin+2*$MezzaRiga] $tipo $font_enorme 
	
         	set testo [descr:modulo $tipo]	;# descrizione del modulo
	
         	set xi [posiz:testo:centrato\
				$xlinea2 $xf_pagina $testo $pixellartesto]
         	crea:testo $canvMods\
         		$xi [expr $ysec_lin+4*$MezzaRiga] $testo $font_testi 
	 }

# TESTI PER LE NOTE 

         set testo "$tx_note"

	 if { $tipovis==1 } {
         	crea:testo $canvMods\
         		[expr $xi_scrit+$distscrtesto]\
         		[expr $yterza_lin+$dist_testilinee]\
         		$testo $font_titfp
	 } else {
         	crea:testo $canvMods\
         		[expr $xi_scrit+$distscrtesto]\
         		[expr $yterza_lin+$distscrtesto+$pixelalttitfp/2]\
         		$testo $font_titfp
	 }
   
# DISEGNO DEI BLOCCHI CHE COMPONGONO IL MODULO

         Crea:Modulo $canvMods $tipo $xi_pagina $yquarta_lin

# titolo di fine pagina

         crea:titolo:finepag $canvMods $tipoStp

         incr numpagine			;# conto le pagine 

# creazione del file postscript per ogni modulo

         if { $tipovis!=1 } {
		#
		#  per ora la produzione del post-script solo
		#  quando premo VISUALIZZA
		#
         	$canvMods addtag tagMod all

		if { $tipovis==3 } {
         		set nomefile "$filestampa$NumeroDellaPagina" 
			crea:postscript $canvMods $nomefile
		}
	 }
      }
   }
 }  ;# endfor scheda....
#
# FINE DELLA GESTIONE SCHEDE SPECIALI (MPP3,UDC E COMPAGNIA BELLA)
#

 cerca:io:speciali $canvMods $Pagina_Moduli

 set NumPagMSD $numpagine

 if { $tipovis!=1 && $tipovis!=3} {
	pack $frameModuli.canvMod -side left -fill both -expand true
	pack $frameModuli -side left -fill both -expand true
 }

 if { $tipovis==3 } {
	if [winfo exists .topModuli] {
		destroy .topModuli
 	}
 }
}



############################################################################
##                                                                        ##
## STAMPA:CROSS - crea canvas e pagine di stampa "cross reference"        ##
##                                                                        ##
##      Input: tipovis = 1 --> produce vari file post-script contenenti   ##
##                             la CrossRef (num. da 10 in su)             ##
##             tipovis = 2 --> visualizza solo il canvas                  ##
##                                                                        ##
############################################################################

proc stampa:cross { } {

	global	indiceincorso
 	global	OK_INITIALIZE SpesMed NumeroDellaPagina Nomfil_debug
 	global	EditPrefs tipovis DX_LINEASEP
 	global	str_type str_scope str_retain str_location
 	global	str_pied str_modulo str_ioline str_initval 
 	global	tx_des tx_typ x_typ
 	global	tx_sco tx_ret pixellartesto dimfogliox2 dimfoglioy2
 	global	nolines tx_loc tx_mod tx_lin tx_ini tx_iol
 	global	numpagine x_sco x_ret x_loc x_mod x_iol
 	global	font_var font_testi font_descr font_lines font_valore
 	global	dimfogliox dimfoglioy NumeroVariabili yragg
 	global	cornice spesscor distscrtesto yprima_lin
 	global	PuntiPerRiga numelin num_line nomefileplc font_initval
 	global	y_titolofp nomvar com_variabile
 	global	canvCross posx_testo matchlinesPerRiga
 	global	tipoStp font_titolino pixellargrande TipoCrossR topCrossRef
 	global	xi_pagina xf_pagina yi_pagina yf_pagina tx_fine tx_notfo
 	global	xi_scrit xf_scrit yi_scrit yf_scrit spaziodalfondo
 	global	punti_x punti_y punti_per_mm dist_testilinee SaltaFormFeed


 if { $tipovis==1 || $tipovis==3 } {
	coord:pagina $dimfogliox $dimfoglioy
 } else {
	coord:pagina $dimfogliox2 $dimfoglioy2
 }
 
 posiz:titolo:finepag		;# calcola posizione titolo fine pagina
 set tipoStp $TipoCrossR	;# stringa generica per settare l'argomento

 set numpagine	1
 set numpag	0
 set yprima_lin  $yi_pagina
 set yragg       $yprima_lin
 set newpage     1
 set ultima      0
 set height_att	$dimfoglioy

 if {$tipovis!=1} {
	if [winfo exists .topCrossRef] {
		destroy .topCrossRef
 	}

 	toplevel .topCrossRef
 	wm title .topCrossRef "STAMPA CROSS REFERENCE"
        wm protocol .topCrossRef WM_DELETE_WINDOW procx

 	set textCross	[text .topCrossRef.frmCro]
 	set framCross	[frame .topCrossRef.frmCro2]
	set canvCross	[canvas $textCross.canvCros\
			-width $dimfogliox -height $dimfoglioy\
			-xscrollcommand [list $textCross.xscroll set] \
	 		-yscrollcommand [list $textCross.yscroll set]]

	scrollbar $textCross.yscroll -orient vertical\
					-command [list $canvCross yview]
	scrollbar $textCross.xscroll -orient horizontal\
					-command [list $canvCross xview]
 } else {
	set canvCross "ProvaCNI"
 }

 #
 # LOOP PRINCIPALE DI VISUALIZZAZIONE DELLA VARIABILI
 #

 for {set i 0} {$i<$NumeroVariabili} {incr i} {

    # ANALISI DELLA SINGOLA VARIABILE: $i

    set k             $nolines($i)                    ;# numero di match lines
    set righeperlinee [expr $k/$matchlinesPerRiga]    ;# righe per match lines

    if { [expr $k%$matchlinesPerRiga] != 0 } {
    	incr righeperlinee
    }

    if { [expr $NumeroVariabili-1] == $i } {
	set ultima 1
    }

    # CALCOLA LE COORDINATE Y PER DISEGNO LINEE DEL RETICOLATO

    set yprima_lin  $yragg
    set ysecon_lin  [expr $yprima_lin+$PuntiPerRiga]
    set yterza_lin  [expr $ysecon_lin+$PuntiPerRiga]
    set yquart_lin  [expr $yterza_lin+$PuntiPerRiga]
    set yquint_lin  [expr $yquart_lin+$PuntiPerRiga]

    if { $tipovis==1 } {
    	set ysesta_lin  $yquint_lin
	set xmod_lin    [expr $xi_pagina+$DX_LINEASEP]
	set xlin_lin    [expr $xmod_lin-25*$punti_per_mm]
    	set xiii	[expr $xmod_lin+$distscrtesto]
    	set xraa	[expr 10+$xf_pagina]
    	set xfmax	[expr $xf_pagina-1]

    	set fatt        12
    	while { [expr $xiii+$fatt*$distscrtesto*10] > $xfmax } {
		incr fatt -1
    	}
	set fattore $fatt

    } else {
    	set ysesta_lin  [expr $yquint_lin+$PuntiPerRiga]
    	set xmod_lin    [expr $xi_pagina+$DX_LINEASEP]
    	set xlin_lin    [expr $xmod_lin-10*$distscrtesto]
    	set xfmax	[expr $xf_pagina-1]
    	set xiii	[expr $xmod_lin+$distscrtesto]

    	set fatt        12
    	while { [expr $xiii+$fatt*$distscrtesto*10] > $xfmax } {
		incr fatt -1
    	}
	set fattore $fatt
    }

    set coxli(0)  [expr $xmod_lin+$distscrtesto]		;#coord. lin. 1
    set coxli(1)  [expr $coxli(0)+$fattore*$distscrtesto]	;#coord. lin. 2
    set coxli(2)  [expr $coxli(1)+$fattore*$distscrtesto]	;#coord. lin. 3
    set coxli(3)  [expr $coxli(2)+$fattore*$distscrtesto]	;#coord. lin. 4
    set coxli(4)  [expr $coxli(3)+$fattore*$distscrtesto]	;#coord. lin. 5
    set coxli(5)  [expr $coxli(4)+$fattore*$distscrtesto]	;#coord. lin. 6
    set coxli(6)  [expr $coxli(5)+$fattore*$distscrtesto]	;#coord. lin. 7
    set coxli(7)  [expr $coxli(6)+$fattore*$distscrtesto]	;#coord. lin. 8
    set coxli(8)  [expr $coxli(7)+$fattore*$distscrtesto]	;#coord. lin. 9
    set coxli(9)  [expr $coxli(8)+$fattore*$distscrtesto]	;#coord. lin. 10

    set x_typ	$coxli(0)		;# X scrittura TYPE
    set x_sco	$coxli(2)		;# X scrittura SCOPE
    set x_ret	$coxli(4)		;# X scrittura RETAIN

    if { $tipovis==1 || $tipovis==3 } {
    	set x_loc	$coxli(6)	;# X scrittura LOCATION
    	set x_iol	$coxli(8)	;# X scrittura I/O LINE (eventuale)
    	set x_mod	$coxli(9)	;# X scrittura MODULE (eventuale)
    } else {
    	set x_loc	$coxli(5)	;# X scrittura LOCATION
    	set x_iol	$coxli(7)	;# X scrittura I/O LINE (eventuale)
    	set x_mod	$coxli(8)	;# X scrittura MODULE (eventuale)
    }

    set yragg   [expr $yquint_lin+$PuntiPerRiga*$righeperlinee]
    set yfinpa	[expr $y_titolofp-$spaziodalfondo]

    if {$righeperlinee==0} {
	set yragg   [expr $yragg+$PuntiPerRiga]
    }
    if {$ultima==1} {
	set yfinpa [expr $yfinpa-$spaziodalfondo]
    }


    if {$newpage==0} {

	#
	# CONTROLLA SE E` STATA COMPLETATA UNA PAGINA
	#

	if { $yragg > $yfinpa } {

		if { $tipovis==1 || $tipovis==3 } {
			set newpage 1
		} else {
			set height_att [expr $height_att+$dimfoglioy]
		}
	}
    }

    if {$newpage==1} {

	if {$tipovis==1} {

		#
		# STAMPA DELLA CROSS REFERENCE SOTTO WINDOWS
		#

 		Next_Page
		
		if {$cornice} {
			crea:cornice $canvCross\
				$xi_pagina $yi_pagina $xf_pagina $yf_pagina
		}

		incr numpag	;# INCREMENTA NUMERO PAGINA DA SCRIVERE

		crea:titolo:finepag $canvCross $tipoStp

		set posx_testo  $xi_scrit

		set yprima_lin	$yi_pagina
		set yappo	[inizio:crossr $canvCross]
		set yprima_lin  [expr $yappo+$PuntiPerRiga/2]

		if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xmod_lin,$yappo,$xmod_lin,$yprima_lin
		}

		set yragg	$yprima_lin
		set ysecon_lin  [expr $yprima_lin+$PuntiPerRiga]
		set yterza_lin  [expr $ysecon_lin+$PuntiPerRiga]
		set yquart_lin  [expr $yterza_lin+$PuntiPerRiga]
		set yquint_lin  [expr $yquart_lin+$PuntiPerRiga]
		set ysesta_lin  $yquint_lin
	} else {
	   if { $tipovis==3 } {

		#
		# STAMPA DELLA CROSS REFERENCE SOTTO UNIX
		#

		if { $numpag!=0 } {
			$canvCross addtag tagCross all

			set filestampa "$Nomfil_debug$NumeroDellaPagina"
			crea:postscript $canvCross $filestampa
		}

		incr numpag	;# INCREMENTA NUMERO PAGINA DA SCRIVERE

		if [info exist framCross] {
			$canvCross delete tagCross
		}

 		set yprima_lin  $yi_pagina
 		set yragg       $yprima_lin

		if {$cornice} {
			crea:cornice $canvCross\
				$xi_pagina $yi_pagina $xf_pagina $yf_pagina
		}

		crea:titolo:finepag $canvCross $tipoStp

		set posx_testo  $xi_scrit

		set yprima_lin	$yi_pagina
		set yappo	[inizio:crossr $canvCross]
		set yprima_lin  [expr $yappo+$PuntiPerRiga/2]

		$canvCross create line $xmod_lin $yappo $xmod_lin $yprima_lin

		set yragg	$yprima_lin
		set ysecon_lin  [expr $yprima_lin+$PuntiPerRiga]
		set yterza_lin  [expr $ysecon_lin+$PuntiPerRiga]
		set yquart_lin  [expr $yterza_lin+$PuntiPerRiga]
		set yquint_lin  [expr $yquart_lin+$PuntiPerRiga]
		set ysesta_lin  [expr $yquint_lin+$PuntiPerRiga]

	    } else {

		#
		# SEMPLICE VISUALIZZAZIONE DELLA CROSS REFERENCE
		#

		set posx_testo  $xi_scrit

		set yprima_lin	$yi_pagina

		set yappo	[inizio:crossr $canvCross]
		set yprima_lin  [expr $yappo+$PuntiPerRiga/2]

		$canvCross create line\
				$xmod_lin $yappo $xmod_lin $yprima_lin

		set yragg	$yprima_lin
		set ysecon_lin  [expr $yprima_lin+$PuntiPerRiga]
		set yterza_lin  [expr $ysecon_lin+$PuntiPerRiga]
		set yquart_lin  [expr $yterza_lin+$PuntiPerRiga]
		set yquint_lin  [expr $yquart_lin+$PuntiPerRiga]
		set ysesta_lin  [expr $yquint_lin+$PuntiPerRiga]
	    }
	}
    }

    set yrighe		[expr $ysesta_lin+$dist_testilinee]
    set posx_testo	[expr $xi_scrit+$pixellargrande]
    set posx_lines	[expr $xlin_lin+2*$distscrtesto]

    set yragg		[expr $ysesta_lin+$PuntiPerRiga*$righeperlinee]

    if {$righeperlinee==0} {
	set yragg   [expr $yragg+$PuntiPerRiga]
    }

    #
    # CREA IL "RETICOLATO" PER TUTTI I DATI VARIABILE
    #

    if { $tipovis==1 } {
        set yappo [expr $ysesta_lin]
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$yragg,$xf_pagina,$yragg
	PrtLine -S$SpesMed -P$xlin_lin,$yappo,$xmod_lin,$yappo
	PrtLine -S$SpesMed -P$xlin_lin,$yappo,$xlin_lin,$yragg
	PrtLine -S$SpesMed -P$xmod_lin,$yappo,$xmod_lin,$yprima_lin
	}
    } else {
        set yappo [expr $ysesta_lin-4]
    	$canvCross create line $xi_pagina $yragg $xf_pagina $yragg
    	$canvCross create line $xlin_lin $yappo $xmod_lin $yappo
    	$canvCross create line $xlin_lin $yappo $xlin_lin $yragg
    	$canvCross create line $xmod_lin $yappo $xmod_lin $yprima_lin
    }

    set step 5

    if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -T3 -P$xmod_lin,$yappo,$xf_pagina,$yappo -S1
	}
    } else {
    	crea:tratteggio $canvCross $xmod_lin $yappo $step
    }

    set posx_testo2 [expr $xmod_lin+$distscrtesto]
    set lungh_testo [string length $nomvar($i)]

    #
    # SCRIVE NOME VARIABILE e DESCRIZIONE VARIABILE
    #

    if { $tipovis==1 } {
	set y_desvar	[expr $yprima_lin+$dist_testilinee]
	set y_nomvar	[expr $yprima_lin+$PuntiPerRiga]
    } else {
    	set y_desvar	[expr $yprima_lin+$PuntiPerRiga]
	set y_nomvar	[expr $yprima_lin+2*$PuntiPerRiga]
    }

    crea:testo:bold $canvCross $posx_testo $y_nomvar $nomvar($i) $font_var
    crea:testo $canvCross $x_typ $y_desvar $tx_des $font_titolino
    crea:testo:bold $canvCross $coxli(3)\
				$y_desvar $com_variabile($i) $font_descr

    # scrive MATCH LINES

    if { $righeperlinee!=0 } {

	crea:testo $canvCross $posx_lines $yrighe $tx_lin $font_titolino

	for {set k 0} {$k<$nolines($i)} {incr k} {
		set indice [expr $k%$matchlinesPerRiga]
		set app [expr $k/$matchlinesPerRiga]
		set yappo [expr $yrighe+$app*$PuntiPerRiga]
		crea:testo:bold $canvCross\
			$coxli($indice) $yappo $numelin($i,$k) $font_lines
	}
    } else {
	crea:testo:bold $canvCross $posx_lines $yrighe $tx_notfo $font_lines
    }

    if { $tipovis==1 } {
    	set yinitva [expr $y_desvar+$PuntiPerRiga]
    	set ytestat [expr $yinitva+$PuntiPerRiga]
    	set yvalore [expr $ytestat+$PuntiPerRiga]
    } else {
    	set ytestat [expr $yterza_lin+$PuntiPerRiga]
    	set yvalore [expr $ytestat+$PuntiPerRiga]
    	set yinitva [expr $y_desvar+$PuntiPerRiga]
    }

    # scrive TYPE

    crea:testo $canvCross $x_typ $ytestat $tx_typ $font_titolino
    crea:testo:bold $canvCross $x_typ $yvalore $str_type($i) $font_valore

    # scrive SCOPE

    crea:testo $canvCross $x_sco $ytestat $tx_sco $font_titolino
    crea:testo:bold $canvCross $x_sco $yvalore $str_scope($i) $font_valore

    # scrive RETAIN

    crea:testo $canvCross $x_ret $ytestat $tx_ret $font_titolino
    crea:testo:bold $canvCross $x_ret $yvalore $str_retain($i) $font_valore

    # scrive LOCATION

    crea:testo $canvCross $x_loc $ytestat $tx_loc $font_titolino
    crea:testo:bold $canvCross $x_loc $yvalore $str_location($i) $font_valore

    if { [string tolower $str_location($i)] != "memory" &&\
	[string tolower $str_type($i)] == "bool" } {

    	# scrive MODULE

    	set piedinus $str_pied($i)

    	if { $piedinus >= 0 && $piedinus < 100 } {

		set str_byt [expr $piedinus /8 ]
		set str_bit [expr $piedinus %8 ]

		crea:testo $canvCross $x_mod $ytestat $tx_mod $font_titolino
		crea:testo $canvCross $x_iol $ytestat $tx_iol $font_titolino
		crea:testo:bold $canvCross $x_mod $yvalore\
			"$str_modulo($i)/$str_byt/$str_bit" $font_valore
		crea:testo:bold $canvCross $x_iol $yvalore\
			$str_ioline($i) $font_valore
    	}
    }

    # scrive INIT VALUE

    set testap $str_initval($i)

    crea:testo $canvCross $x_typ $yinitva $tx_ini $font_titolino
    crea:testo:bold $canvCross\
			$coxli(3) $yinitva $str_initval($i) $font_initval

    set newpage 0
 }

 #
 # FINE DEL LOOP PRINCIPALE DI VISUALIZZAZIONE DELLA VARIABILI
 #

 set yfinale [fine:crossr $canvCross $yragg]

 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
	PrtLine -S$SpesMed -P$xi_pagina,$yragg,$xf_pagina,$yragg
	}
 } else {
	$canvCross create line $xi_pagina $yragg $xf_pagina $yragg
 }

 if { $tipovis!=1 && $tipovis!=3 } {

 	set appog [expr $yfinale-$yi_pagina+2*$PuntiPerRiga]
  	$canvCross configure -scrollregion\
		" $xi_pagina $yi_pagina $dimfogliox $appog " 
	if { $yragg > $dimfoglioy2 } {
		pack $textCross.yscroll -side right -fill y
	}

 	pack $canvCross -side left -fill both -expand true
 	pack $framCross -side top -fill x -pady 2
 	pack $textCross -side bottom -fill x -expand true
 }

 if { $tipovis==3 } {
	set filestampa "$Nomfil_debug$NumeroDellaPagina"
	crea:postscript $canvCross $filestampa

	if [winfo exists .topCrossRef] {
		destroy .topCrossRef
 	}
 }
}



##################################################################
#
#	NEXT_PAGE:	Esegue un Form-Feed (se necessario)
#
##################################################################
proc Next_Page { } {

	global	indiceincorso
	global	SaltaFormFeed

 if { $SaltaFormFeed==0 } {
	if { $indiceincorso==1 } {
	PrtFFeed
	}
 } else {
	set SaltaFormFeed 0
 }
}


##################################################################
#	Procedura opz_outwin : esce dalla finestra opzioni
##################################################################
#
#	input : tipout  >> tipo di out (0 ok / 1 annulla)
#		outwid  >> widget da cui uscire

proc opz_outwin {tipout outwid} {
#
# variabili di ritorno configurazione
#
global PrtOpt
global PRT_OPZ_W
global PRT_OPZ_SAVE
#
global prt_opz_change   ;# flag di opzioni cambiate per save ( yes / no )

 if {$tipout==0} {
      #
      # salvo i valori nell'array generale di stampa
      #
      array set PrtOpt [array get PRT_OPZ_W]
      set prt_opz_change yes	;# set flag di opzioni cambiate
      # newopz
  }

  if {$tipout==1} {
      array set PrtOpt [array get PRT_OPZ_SAVE]
      # newopz
  }

 destroy $outwid
}

##################################################################
#	Procedura opz_default : gestione finestra opzioni
##################################################################
#
#	Ricarica i valori di default delle opzioni
#
#	input : tipcal  >> tipo di chiamata : o Apertura finestra
#					      c tasto conferma premuto
#					      a tasto annulla premuto
#		babbo     >> widget chiamante
#		wid1,wid2 >> widget da abilitare/disabilitare
#

proc opz_default {tipcal babbo wid1 wid2 } {
#
# variabili di ritorno configurazione
#
global EditPrefs
global PrtOpt
global PRT_OPZ_W
global Home tcl_platform
global ProjectConf
#
#
#	Test sul tipo di chiamata ...
#
 switch $tipcal {
	o {

	   ;# Apro la finestra di richiesta conferma

	   ;# se finestra gia' aperta eseguo solo un raise ...

 	    if [ winfo exist .topcnfd ] {
     	      raise .topcnfd
	      focus .topcnfd.frm1.btann
     	      return
    	    }
	   $wid1 configure -state disabled
	   $wid2 configure -state disabled
 	   toplevel .topcnfd
 	   wm protocol .topcnfd WM_DELETE_WINDOW procx  ;# dis.tasto x
 	   wm title .topcnfd [GetString prt_opz_ltig]
	   frame .topcnfd.frm			;# frame label + img
 	   frame .topcnfd.sep -class Rule 	;# frame separatore
 	   label .topcnfd.frm.lbim -justify center \
 	   -image [ image create photo -file $EditPrefs(xpmatt)]
           label .topcnfd.frm.lbt -justify center -text [GetString prt_opz_lcnf]
           tixButtonBox .topcnfd.frm1 -orientation horizontal
           .topcnfd.frm1 add ok  -image ok -font $EditPrefs(windowfont) \
		   -command "opz_default c $babbo $wid1 $wid2"
           .topcnfd.frm1 add cancel -image cancel -font $EditPrefs(windowfont) \
		   -command "opz_default a $babbo $wid1 $wid2"
           CniBalloon [.topcnfd.frm1 subwidget ok] [GetString OK] \
             $EditPrefs(windowfont) 500
           CniBalloon [.topcnfd.frm1 subwidget cancel] [GetString CANCEL] \
             $EditPrefs(windowfont) 500

	   pack .topcnfd.frm.lbim -side left -padx 20 -anchor w
	   pack .topcnfd.frm.lbt -side left -padx 20 -anchor w
	   pack .topcnfd.frm -pady 15 -anchor c -side top

	   pack .topcnfd.frm1 -side top -fill x
	   focus [.topcnfd.frm1 subwidget cancel]
	  }

	a {	;# premuto tasto annulla
	   $wid1 configure -state normal
	   $wid2 configure -state normal
	   focus $wid2
 	   destroy .topcnfd
	  }

	c {	;# premuto tasto conferma ricarico array di default e var.


	   ;# carico i valori di default dall'array generale di stampa

	   source "$ProjectConf"			;#ricarica default
 	   array set PrtOpt [array get EditPrefs]
 	   array set PRT_OPZ_W [array get PrtOpt]
	   $wid1 configure -state normal
	   $wid2 configure -state normal
	   focus $wid2
 	   destroy .topcnfd
	   # delnewopz					;#cancella new options
	  }
 }
}


####################################
#	Procedura :  procx
####################################

#procedura chiamata quando si preme il pulsanta "x" in alto a dx

#se non l'azione di default viene sovrascritta dalle azioni di questa
#funzione.(se la funzione non fa nulla allora il tasto e' come disab.
#

proc procx {} {
}

#############################################################
#
#	pannello opzioni di stampa
#
#############################################################

proc PrintOptions {} {
#
# variabili di ritorno configurazione
#
  global tcl_platform
  global PrtOpt PRT_OPZ_W PRT_OPZ_SAVE
  global str_form formato
  global tipi_foglio altezza_custom larghezza_custom
  global tipi_foglio larghezza_foglio altezza_foglio
  global ima_labels gallo_labels opt_labels
  global EditPrefs

#
# carico i valori di default dall'array generale di stampa all'array di lavoro
#
 array set PRT_OPZ_W [array get PrtOpt]
 array set PRT_OPZ_SAVE [array get PrtOpt]

#
# test se la finestra e' gia' aperta , in tal caso eseguo solo un raise ...
#
 if [ winfo exist .topopz ] {
     raise .topopz
     return
    }

 toplevel .topopz
 wm protocol .topopz WM_DELETE_WINDOW procx  ;# disabilito il tasto x
 wm title .topopz [GetString prt_opz_ltig]
 
 tixNoteBook .topopz.prtopz 

#
# frame & bottoni per bottoni di ok/annulla/reload default
#
 tixButtonBox .topopz.frmbnt -orientation horizontal
 .topopz.frmbnt add ok  -image ok -font $EditPrefs(windowfont) \
     -command "opz_outwin 0 .topopz"

 .topopz.frmbnt add cancel -image cancel -font $EditPrefs(windowfont) \
     -justify center -command "opz_outwin 1 .topopz"

 .topopz.frmbnt add default -image default -font $EditPrefs(windowfont) \
     -command "opz_default o .topopz [.topopz.frmbnt subwidget ok] \
      [.topopz.frmbnt subwidget cancel]"

 CniBalloon [.topopz.frmbnt subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
 CniBalloon [.topopz.frmbnt subwidget default] [GetString prt_opz_lbrel] \
      $EditPrefs(windowfont) 500
 CniBalloon [.topopz.frmbnt subwidget cancel] [GetString CANCEL] \
      $EditPrefs(windowfont) 500

 .topopz.prtopz add pag -label [GetString prt_opz_pag] -underline 1 
 .topopz.prtopz add gen -label [GetString prt_opz_gen] -underline 1 
 .topopz.prtopz add doc -label [GetString prt_opz_doc] -underline 1 
 if { $tcl_platform(platform) == "unix" } {
 	.topopz.prtopz add frm -label [GetString prt_opz_for] -underline 1 
 }

#
#	Widget per Selezione Formato (SOLO SE UNIX!!!!!!!!!!!!!)
#
 if { $tcl_platform(platform) == "unix" } {

	#
	#	Primo frame: intestazione pagina
	#
 	set frame [.topopz.prtopz subwidget frm]

 	frame $frame.sepa1 -class Rule		;# frame "separatore"
 	frame $frame.sepa2 -class Rule		;# frame "separatore"

 	frame $frame.fri		;#frame "intestazione"
 	label $frame.fri.lgpag -justify center -text [GetString prt_opz_for_gen]

 	frame $frame.fr4		;#frame "dim. formato standard"
 	label $frame.fr4.lgpag -justify center

 	frame $frame.fr3		;#frame "seleziona dim."
 	label $frame.fr3.lgpag -justify center\
			-text  [GetString prt_opz_for_sel]

#
# Selezione formato standard
#
 	frame $frame.frc	;#frame prima sezione (tasto selezione foglio)

#set prima_volta 0
#
#	if { [info exists formato]==0 } {
#		set prima_volta 1
 		setta:formati:stampa
#		set formato_default A4
#		set formato $gallo_labels($formato_default)
# 	}

	set save_formato $PRT_OPZ_W(prt_foglio)

 	tixOptionMenu $frame.frc.om\
	-variable PRT_OPZ_W(prt_foglio)\
	-options {
	    menubutton.width 10
	    menubutton.height 1
	}

 	foreach opt $tipi_foglio {
		set disegno1 [image create photo -file $ima_labels($opt)]
		set disegno2\
		[image create compound -window [$frame.frc.om subwidget menu] ]
		$disegno2 add line
		$disegno2 add image -image $disegno1
		$disegno2 add space -width 6
		$disegno2 add text -text $opt_labels($opt)
		$frame.frc.om add command $gallo_labels($opt) \
			-image $disegno2
    	}

#	set formato_attuale [lindex $save_formato 0]
 	$frame.frc.om configure -value $save_formato

	$frame.frc.om configure -command gestisce_tixcontrol

#
# Selezione dimensioni formato
#

 	frame $frame.fr2	;#frame seconda sezione (sel. dimensione)

 	label $frame.fr2.lab_altez -justify right -text [GetString prt_opz_for_alp]
 	tixControl $frame.fr2.altez\
		-integer false\
		-variable PRT_OPZ_W(prt_altfoglio) \
		-step .1\
	    	-min 100.0 -max 499.9\
		-options {
	      	     	entry.width 5
	       	     	entry.anchor w
	      	     	decr.relief groove
	      	     	incr.relief groove
			}

 	label $frame.fr2.lab_largh -justify right -text [GetString prt_opz_for_lap]
 	tixControl $frame.fr2.largh\
		-integer false\
		-variable PRT_OPZ_W(prt_larfoglio) \
		-step .1\
	    	-min 100.0 -max 499.9\
		-options {
	      	     	entry.width 7
	       	     	entry.anchor w
	      	     	decr.relief groove
	      	     	incr.relief groove
			}

	#if { $prima_volta==1 } {
		gestisce_tixcontrol $PRT_OPZ_W(prt_foglio)
	#}

#
#	pack con disposizione dei widget nel modo corretto !!
#
 	pack $frame.fri -fill x -side top
 	pack $frame.fri.lgpag -side top -pady 20
 	pack $frame.sepa1 -fill x -side top
 	pack $frame.frc -fill x -side top -pady 5
 	pack $frame.frc.om -side top
 	pack $frame.fr4 -fill x -side top
 	pack $frame.fr4.lgpag -side top -pady 5
 	pack $frame.sepa2 -fill x -side top -pady 5

 	pack $frame.fr3 -fill x -side top
 	pack $frame.fr3.lgpag -side top -pady 5

 	grid $frame.fr2.lab_altez\
		-row 1 -column 1 -sticky e -pady 15 -padx 10
 	grid $frame.fr2.altez -row 1 -column 2 -pady 15 -padx 10
 	grid $frame.fr2.lab_largh\
		-row 1 -column 3 -sticky e -pady 15 -padx 10
 	grid $frame.fr2.largh -row 1 -column 4 -pady 15 -padx 10
 	pack $frame.fr2 -fill x -side top

 	focus $frame
 }

#
#	widget per selezione pagina
#

 set frame [.topopz.prtopz subwidget pag]
#
# frame per comporre finestra
#
 frame $frame.rulei -class Rule ;# frame che rappresenta un "separatore" int
 frame $frame.fri               ;#frame "intestazione"
 frame $frame.frc               ;#frame centrale
 frame $frame.rules -class Rule ;# frame che rappresenta un "separatore" sez.
 frame $frame.frs               ;#frame seconda sezione
#
# label intestazione pagina 
#
 label $frame.fri.lgpag -justify center -text [GetString prt_opz_pag_gen]
#
# prima sezione : sel. margini
#
 label $frame.frc.lmardxsx -justify right -text [GetString prt_opz_pag_mdxsx]
 tixControl $frame.frc.mardxsx -min 0 -max 3 -variable PRT_OPZ_W(prt_marsxdx) \
	    -min 0 -max 20 -options {
		      		     entry.width 2
	              		     entry.anchor e
		      		     decr.relief groove
		      		     incr.relief groove
	             		    }
 label $frame.frc.lmarasx -justify right -text [GetString prt_opz_pag_msx]
 tixControl $frame.frc.marasx -min 0 -max 3 -variable PRT_OPZ_W(prt_marasx) \
	    -min 0 -max 20 -options {
		      		     entry.width 2
		      		     entry.anchor e
		      		     decr.relief groove
		      		     incr.relief groove
		     		    }
#
#  Seconda sezione : selezione contorno pagina
#
 label $frame.frs.lscorni -justify center -text [GetString prt_opz_pag_scor]
 radiobutton $frame.frs.corni -anchor w -value a \
		-variable PRT_OPZ_W(prt_specorn) -text [GetString prt_opz_pag_cora] -selectcolor $EditPrefs(togglecolor)
 radiobutton $frame.frs.scornis -anchor c -value s \
		-variable PRT_OPZ_W(prt_specorn) -text [GetString prt_opz_pag_cors] -selectcolor $EditPrefs(togglecolor)
 radiobutton $frame.frs.scornim -anchor c -value m \
		-variable PRT_OPZ_W(prt_specorn) -text [GetString prt_opz_pag_corm] -selectcolor $EditPrefs(togglecolor)
 radiobutton $frame.frs.scornil -anchor e -value l \
		-variable PRT_OPZ_W(prt_specorn) -text [GetString prt_opz_pag_corl] -selectcolor $EditPrefs(togglecolor)
#
#	pack con disposizione dei widget nel modo corretto !!
#
 pack $frame.fri.lgpag -side top -pady 15

 grid $frame.frc.lmardxsx -row 1 -column 0 -sticky e -pady 10 -padx 15
 grid $frame.frc.mardxsx -row 1 -column 1 -pady 10 -padx 15
 grid $frame.frc.lmarasx -row 2 -column 0 -sticky e -pady 10 -padx 15
 grid $frame.frc.marasx -row 2 -column 1 -pady 10 -padx 15

 grid $frame.frs.lscorni -row 0 -column 0 -columnspan 4 -pady 10 -padx 15
 grid $frame.frs.corni -row 1 -column 0 -pady 6 -padx 15
 grid $frame.frs.scornis -row 1 -column 1 -pady 6 -padx 15
 grid $frame.frs.scornim -row 1 -column 2 -pady 6 -padx 15
 grid $frame.frs.scornil -row 1 -column 3 -pady 6 -padx 15
 pack $frame.fri -fill x -side top
 pack $frame.rulei -fill x -side top
 pack $frame.frc -fill x -side top
 pack $frame.rules -fill x -side top
 pack $frame.frs -fill x -side top
 focus $frame


#
#	widget per selezione generalita'
#
 set frame [.topopz.prtopz subwidget gen]
#
# frame per comporre finestra
#
 frame $frame.rulei -class Rule ;# frame che rappresenta un "separatore" int
 frame $frame.fri               ;#frame "intestazione"
 frame $frame.frc               ;#frame centrale
 frame $frame.rules -class Rule ;# frame che rappresenta un "separatore" sez.
 frame $frame.frs               ;#frame basso
#
# label intestazione pagina 
#
 label $frame.fri.lggen -justify center -text [GetString prt_opz_gen_gen]
#
# sezione selezione varie
#
 label $frame.frc.lragso -justify right -text [GetString prt_opz_gen_ragso]
 entry $frame.frc.ragso -width 40 -textvariable PRT_OPZ_W(prt_ragso)
 label $frame.frc.lnote -justify right -text [GetString prt_opz_gen_note]
 entry $frame.frc.note -width 40  -textvariable PRT_OPZ_W(prt_note)
 label $frame.frc.llogo -justify right -text [GetString prt_opz_gen_logo]
 entry $frame.frc.logo -width 40  -textvariable PRT_OPZ_W(prt_logo)
#
#  Seconda sezione : selezione tipo data
#
 label $frame.frs.lstipda -justify center -text [GetString prt_opz_gen_data]
 radiobutton $frame.frs.datgm -anchor w -value g \
		-variable PRT_OPZ_W(prt_formdata) -text [GetString prt_opz_gen_gm] -selectcolor $EditPrefs(togglecolor)
 radiobutton $frame.frs.datmg -anchor c -value m \
		-variable PRT_OPZ_W(prt_formdata) -text [GetString prt_opz_gen_mg] -selectcolor $EditPrefs(togglecolor)
#
#	pack con disposizione dei widget nel modo corretto !!
#
 pack $frame.fri.lggen -side top -pady 15

 grid $frame.frc.lragso -row 3 -column 0 -sticky e -pady 10 -padx 15
 grid $frame.frc.ragso -row 3 -column 1 -pady 10 -padx 15
 grid $frame.frc.lnote -row 4 -column 0 -sticky e -pady 10 -padx 15
 grid $frame.frc.note -row 4 -column 1 -pady 10 -padx 15
 grid $frame.frc.llogo -row 5 -column 0 -sticky e -pady 10 -padx 15
 grid $frame.frc.logo -row 5 -column 1 -pady 10 -padx 15

 grid $frame.frs.lstipda -row 0 -column 0 -columnspan 4 -pady 10 -padx 15
 grid $frame.frs.datgm -row 1 -column 1 -pady 6 -padx 15
 grid $frame.frs.datmg -row 1 -column 2 -pady 6 -padx 15

 pack $frame.fri -fill x -side top
 pack $frame.rulei -fill x -side top
 pack $frame.frc -fill x -side top
 pack $frame.rules -fill x -side top -pady 4
 pack $frame.frs -fill x -side top -pady 6              ;#frame basso

#
#	widget per selezione documentazione
#
 set frame [.topopz.prtopz subwidget doc]
#
# frame per comporre finestra
#
 frame $frame.rulei -class Rule ;# frame che rappresenta un "separatore" int
 frame $frame.fri               ;#frame "intestazione"
 frame $frame.frc               ;#frame centrale
#
# label intestazione pagina 
#
 label $frame.fri.lgdoc -justify center -text [GetString prt_opz_doc_gen]
#
# sezione selezione documentazione
#
 label $frame.frc.lprgil -text [GetString prt_opz_doc_prgil] -justify right
 checkbutton $frame.frc.prgil -variable PRT_OPZ_W(prt_progil) -selectcolor $EditPrefs(checkcolor)

 label $frame.frc.lprgld -text [GetString prt_opz_doc_prgld] -justify right
 checkbutton $frame.frc.prgld -variable  PRT_OPZ_W(prt_progld) -selectcolor $EditPrefs(checkcolor)

 label $frame.frc.ltabiu -text [GetString prt_opz_doc_tabiu] -justify right
 checkbutton $frame.frc.tabiu -variable PRT_OPZ_W(prt_tabiu) -selectcolor $EditPrefs(checkcolor)

 label $frame.frc.lcross -text [GetString prt_opz_doc_cross] -justify right
 checkbutton $frame.frc.cross -variable PRT_OPZ_W(prt_crossr) -selectcolor $EditPrefs(checkcolor)

 label $frame.frc.lcoper -text [GetString prt_opz_doc_coper] -justify right
 checkbutton $frame.frc.coper -variable PRT_OPZ_W(prt_copert) -selectcolor $EditPrefs(checkcolor)

 label $frame.frc.lindic -text [GetString prt_opz_doc_indic] -justify right
 checkbutton $frame.frc.indic -variable PRT_OPZ_W(prt_indice) -selectcolor $EditPrefs(checkcolor)

#
#	pack con disposizione dei widget nel modo corretto !!
#
 pack $frame.fri.lgdoc -side top -pady 15

 grid $frame.frc.lprgil -row 3 -column 0 -sticky e -pady 5
 grid $frame.frc.prgil -row 3 -column 1 -pady 5
 grid $frame.frc.lprgld -row 3 -column 2 -sticky e -pady 5
 grid $frame.frc.prgld -row 3 -column 3 -pady 5
 grid $frame.frc.ltabiu -row 4 -column 0 -sticky e -pady 5
 grid $frame.frc.tabiu -row 4 -column 1 -pady 5 -padx 15
 grid $frame.frc.lcross -row 4 -column 2 -sticky e -pady 5
 grid $frame.frc.cross -row 4 -column 3 -pady 5 -padx 15
 grid $frame.frc.lcoper -row 5 -column 0 -sticky e -pady 5
 grid $frame.frc.coper -row 5 -column 1 -pady 5 -padx 15
 grid $frame.frc.lindic -row 5 -column 2 -sticky e -pady 5
 grid $frame.frc.indic -row 5 -column 3 -pady 5

 pack $frame.fri -fill x -side top
 pack $frame.rulei -fill x -side top
 pack $frame.frc -fill x -side top -pady 6

#
# pack notebook g
#
 pack .topopz.prtopz
#
#	pack bottoni ok,annulla,ricarica
#
 pack .topopz.frmbnt  -fill x
}


############################################################################
#
#	PrintDocAl		stampa documentazione completa
#
############################################################################

proc PrintDocAl { t } {

	global	indiceincorso NumPSbase docall
	global	flag_stampa_pil flag_stampa_pld flag_stampa_msd
	global	flag_stampa_cro flag_stampa_cop flag_stampa_ind
	global	EditPrefs str_pied OK_INITIALIZE
	global	tipovis VariableList punti_per_mm
	global	nomvar com_variabile str_type str_scope str_retain
	global	str_location str_modulo str_ioline str_initval nolines numelin
	global	tx_print tx_printcov tx_printcon
	global	tx_printld tx_printcro tx_printil
	global	NumeroDellaPagina Dir_ps NomePS NumPS
	global	Pagina_Moduli flag_riep
	global	Pagina_Riep_Moduli
	global	Pagina_CrossR
	global	Pagina_ProgIL
	global	Pagina_ProgLD
	global	SaltaFormFeed

 set_tipovis
 set docall 1
 set flag_riep 0

 if { $tipovis!=2 } {
 	WaitWindow $t 1
 }

 set ritorno [init:stampa:read:file $t ]

 if { $ritorno==1 } {
		WaitWindow $t 0
		return
 }

 if { $tipovis==1 && $flag_stampa_ind } {
	#
	# lavoro sotto DOS: la stampa avviene direttamente senza
	# produzione di file Postscript: questo implica una fase
	# di "pre-processor" per potere stampare l'indice per
	# prima cosa. Quando la variabile "indiceincorso" vale 1
	# vengono chiamate le stesse procedure della stampa vera
	# e propria inibendo pero` le operazioni sulla stampante
	#
	set indiceincorso 0		;# inibisce operazioni stampante
	set NumeroDellaPagina 2		;# lascia una pagina per l'indice

 	if { $flag_stampa_pil } {
 		set Pagina_ProgIL $NumeroDellaPagina
 		stampa:testo:plc $t
	}
 	if { $flag_stampa_msd } {
 		set Pagina_Moduli $NumeroDellaPagina
 		stampa:moduli
	}
 	if { $flag_stampa_cro } {
 		set Pagina_CrossR $NumeroDellaPagina
 		stampa:cross
	}
 	if { $flag_stampa_pld } {
 		set Pagina_ProgLD $NumeroDellaPagina
 		stampa:ladder:plc
	}
 }

 set indiceincorso 1
 set NumeroDellaPagina 1

 if { $flag_stampa_cop } {
		stampa:copertina
		set SaltaFormFeed 0
 }

 if { $flag_stampa_ind && $tipovis==1 } {
		#
		# ambiente DOS / WINDOWS: la stampa avviene direttamente
		# e l'indice e` gia` stato preparato:
		# la stampa dell'indice avviene prima di stampare il resto
		#
 		stampa:indice
		# incr NumeroDellaPagina	;# gallonzibarbububu
 }

 if { $flag_stampa_ind==0 } {
	set NumeroDellaPagina 0
 }

 set Pagina_ProgIL $NumeroDellaPagina

 if { $flag_stampa_pil } {
 		stampa:testo:plc $t
 }

 set Pagina_Moduli $NumeroDellaPagina

 if { $flag_stampa_msd } {
 		stampa:moduli
 }

 set Pagina_CrossR $NumeroDellaPagina

 if { $flag_stampa_cro } {
 		stampa:cross
 }

 set Pagina_ProgLD $NumeroDellaPagina

 if { $flag_stampa_pld } {
 		stampa:ladder:plc
 }

 if { $flag_stampa_ind && $tipovis==3 } {
		#
		# non e` ambiente DOS / WINDOWS:
		# la stampa dell'indice avviene dopo aver stampato il resto
		#
 		stampa:indice
 }

 if { $flag_stampa_ind } {
 	incr NumPS -1
 }

 if { $tipovis==3 } {
	if { $flag_stampa_ind } {
		set nn $NumPSbase
		if { $flag_stampa_cop } {
               		set kom1\
			 "catch { exec lpr $Dir_ps$NomePS$NumPSbase.ps } app1"
                	eval $kom1
                	set kom2\
			 "catch { exec rm $Dir_ps$NomePS$NumPSbase.ps } app2"
                	eval $kom2
			incr nn
		}
               	set kom1\
			 "catch { exec lpr $Dir_ps$NomePS$NumPS.ps } app1"
                eval $kom1
                set kom2\
			 "catch { exec rm $Dir_ps$NomePS$NumPS.ps } app2"
                eval $kom2
	        for {set n $nn} {$n <$NumPS} {incr n} {
               		set kom1\
			 "catch { exec lpr $Dir_ps$NomePS$n.ps } app1"
                	eval $kom1
                	set kom2\
			 "catch { exec rm $Dir_ps$NomePS$n.ps } app2"
                	eval $kom2
		}
	} else {
	        for {set n $NumPSbase} {$n <$NumPS} {incr n} {
               		set kom1\
			 "catch { exec lpr $Dir_ps$NomePS$n.ps } app1"
                	eval $kom1
                	set kom2\
			 "catch { exec rm $Dir_ps$NomePS$n.ps } app2"
                	eval $kom2
		}
	}
 }

 if { $tipovis==1 && $OK_INITIALIZE!=0 } {
 		PrtStop
 }

 if { $tipovis!=2 } {
		WaitWindow $t 0
 }
}				


#########################################################################
#
# PrintCrossReference    stampa o visualizzazione della cross reference
#
#########################################################################

proc PrintCrossReference { t } {

	global	EditPrefs str_pied OK_INITIALIZE
	global	indiceincorso docall
	global	tipovis VariableList punti_per_mm
	global	nomvar com_variabile str_type str_scope str_retain
	global	str_location str_modulo str_ioline str_initval nolines numelin
	global	NumeroDellaPagina tx_printcro

 set_tipovis
 set docall 0

 if { $tipovis!=2 } {
 	WaitWindow $t 1
 }

 set ritorno [init:stampa:read:file $t ]

 if { $ritorno==1 } {
	WaitWindow $t 0
	return
 }

 set NumeroDellaPagina 0

 stampa:cross

 if { $tipovis==1 && $OK_INITIALIZE!=0 } {
 	PrtStop
 }

 if { $tipovis!=2 } {
 	WaitWindow $t 0
 }
}		;# END PRINT:CROSS:REFERENCE



############################################################################
#
# PrintMsd			stampa connessioni Input/Output	
#
############################################################################

proc PrintMsd { t } {

	global	indiceincorso docall
	global	EditPrefs str_pied OK_INITIALIZE
	global	tipovis VariableList punti_per_mm
	global	nomvar com_variabile str_type str_scope str_retain
	global	str_location str_modulo str_ioline str_initval nolines numelin
	global	NumeroDellaPagina tx_printcon

 set_tipovis
 set docall 0

 if { $tipovis!=2 } {
 	WaitWindow $t 1
 }

 set ritorno [init:stampa:read:file $t ]

 if { $ritorno==1 } {
 	WaitWindow $t 0
	return
 }

 set NumeroDellaPagina 0

 stampa:moduli

 if { $tipovis==1 && $OK_INITIALIZE!=0 } {
 	PrtStop
 }

 if { $tipovis!=2 } {
 	WaitWindow $t 0
 }
}		;# END PRINT:MODULI


############################################################################
#
# PrintIlProgram		stampa programma PLC IL
#
############################################################################
proc PrintIlProgram { t } {

	global	indiceincorso docall
	global	EditPrefs str_pied OK_INITIALIZE
	global	tipovis VariableList punti_per_mm
	global	nomvar com_variabile str_type str_scope str_retain
	global	str_location str_modulo str_ioline str_initval nolines numelin
	global	NumeroDellaPagina tx_printil

 set_tipovis
 set docall 0

 if { $tipovis!=2 } {
 	WaitWindow $t 1
 }

 set ritorno [init:stampa:read:file $t ]

 if { $ritorno==1 } {
 	WaitWindow $t 0
	return
 }

 set NumeroDellaPagina 0

 stampa:testo:plc $t

 if { $tipovis==1 && $OK_INITIALIZE!=0 } {
	if { $indiceincorso==1 } {
 	PrtStop
	}
 }

 if { $tipovis!=2 } {
 	WaitWindow $t 0
 }

}		;# END PRINT:PROGRAMMA:IL



###############################################################
#
#	stampa:copertina
#
###############################################################
proc stampa:copertina { } {

	global	indiceincorso
	global	xi_pagina xf_pagina yi_pagina yf_pagina tipovis
	global	xi_scrit xf_scrit yi_scrit yf_scrit PuntiPerRiga
	global	canvapp dimfogliox dimfoglioy TipoIndice punti_per_mm
	global	flag_stampa_pil flag_stampa_pld flag_stampa_msd
	global	flag_stampa_cro flag_stampa_cop flag_stampa_ind
	global	nomefilesenzaplc dist_testilinee SpesMed distscrtesto
	global	Pagina_Moduli versione vers_prg ragsoc
	global	Pagina_Riep_Moduli font_testibig Nomfil_debug
	global	Pagina_ProgIL Pagina_CrossR cornice Pagina_ProgLD
	global	NumeroDellaPagina NomeBitmap pixellartestobig
	global	font_enorme font_testibig data_sys
	global	tx_docall tx_index tx_ulmo data_ulmo tx_ind_io
 	global	tx_ind_in tx_ind_il tx_ind_ld tx_ind_cr tx_ind_rm
        global	fnt12x18 fnt5x10 fnt4x8 fnt5x7 fnt3x4 font_grande
	global	y_cnidoc tx_cnidoc font_pikkolo fnt4x7 font_enorme2

 if { $tipovis==1 } {

	coord:pagina $dimfogliox $dimfoglioy

	crea:cornice $canvapp $xi_pagina $yi_pagina $xf_pagina $yf_pagina

	crea:testo:bold $canvapp $xi_pagina $y_cnidoc $tx_cnidoc $font_pikkolo

	set dimbitmap [expr 50*$punti_per_mm]

	set xtesto [expr $xi_pagina+20*$punti_per_mm]
	set xnumpa [expr $xtesto+100*$punti_per_mm]
	set ytesto [expr $yi_pagina+20*$punti_per_mm]

	set xfbitmap [expr $xtesto+$dimbitmap]
	set yfbitmap [expr $ytesto+$dimbitmap]

	set ytest2 [expr $ytesto+4*$dist_testilinee]
	set xtest2 [expr $xi_pagina+50*$punti_per_mm]

 	set font_appo $fnt12x18

 	crea:testo:bold $canvapp $xtesto $ytest2 $nomefilesenzaplc $font_appo

	set ytesto [expr $ytesto+30*$punti_per_mm]

 	set font_appo $fnt4x8

	set app   [string length $vers_prg]
 	set app1  [expr $app-2]
 	set app2  [string range $vers_prg 1 $app1]

 	set testo "$versione $app2"

	crea:testo:bold $canvapp $xtesto $ytesto $testo $font_appo

	set ytesto [expr $ytesto+20*$punti_per_mm]

	set testap "$tx_ulmo $data_ulmo"

	crea:testo:bold $canvapp $xtesto $ytesto $testap $font_appo

	set ytesto [expr $ytesto+50*$punti_per_mm]

	set xi [expr $xi_pagina+(($xf_pagina-$xi_pagina)/2)-($dimbitmap/2)]
	set yi $ytesto
	set xf [expr $xi+$dimbitmap]
	set yf [expr $yi+$dimbitmap]
	if { $indiceincorso==1 } {
	  PrtPoli -S$SpesMed -P$xi,$yi,$xf,$yi,$xf,$yf,$xi,$yf
	  if { $NomeBitmap != "" } {
	     PrtBmp $NomeBitmap -C$xi,$yi,$xf,$yf
	  }
	}

	set ytesto [expr $yf+5*$punti_per_mm]

 	set font_appo $fnt3x4

	crea:testo:centrato:bold $xi_pagina $xf_pagina\
						 $ytesto $ragsoc $font_appo

 } else {

 	wm withdraw .
 	toplevel .topCopert
 	wm title .topCopert "STAMPA COPERTINA"
        wm protocol .topCopert WM_DELETE_WINDOW procx
 	set frameCopert [frame .topCopert.frmcop]    ;# conterra' il canvas
 	set canvcop [canvas $frameCopert.canvCopert\
			-width $dimfogliox -height $dimfoglioy]

	coord:pagina $dimfogliox $dimfoglioy

	crea:cornice $canvcop $xi_pagina $yi_pagina $xf_pagina $yf_pagina

	crea:testo:bold $canvcop $xi_pagina $y_cnidoc $tx_cnidoc $font_pikkolo

	set dimbitmap [expr 50*$punti_per_mm]

	set xtesto [expr $xi_pagina+10*$distscrtesto]
	set ytesto [expr $yi_pagina+4*$PuntiPerRiga]
	set ytest2 [expr $ytesto+$dist_testilinee]

 	set font_appo $font_enorme2
 	crea:testo:bold $canvcop $xtesto $ytest2 $nomefilesenzaplc $font_appo

	set ytesto [expr $ytesto+6*$PuntiPerRiga]

	set app   [string length $vers_prg]
 	set app1  [expr $app-2]
 	set app2  [string range $vers_prg 1 $app1]
 	set testo "$versione $app2"

 	set font_appo $font_grande
	crea:testo:bold $canvcop $xtesto $ytesto $testo $font_appo

	set ytesto [expr $ytesto+4*$PuntiPerRiga]

	set testap "$tx_ulmo $data_ulmo"
	crea:testo:bold $canvcop $xtesto $ytesto $testap $font_appo

	set ytesto [expr $ytesto+4*$PuntiPerRiga]

	set xi [expr $xi_pagina+(($xf_pagina-$xi_pagina)/2)-($dimbitmap/2)]
	set yi $ytesto
	set xf [expr $xi+$dimbitmap]
	set yf [expr $yi+$dimbitmap]

	if { $indiceincorso==1 } {
                set nome_troncb [string range $NomeBitmap 0 [expr\
                                     [string first "." $NomeBitmap]-1]]
                set nome_tronc [string range $NomeBitmap 1 [expr\
                                     [string first "." $NomeBitmap]-1]]
                set nome_suff [string range $NomeBitmap [expr\
                                     [string first "." $NomeBitmap]] end]
		set letmin "c"
		set letmai "C"
		set nome1b "$nome_troncb$letmin$nome_suff"
		set nome2b "$nome_troncb$letmai$nome_suff"
		set nome1 "$nome_tronc$letmin$nome_suff"
		set nome2 "$nome_tronc$letmai$nome_suff"
		set NomeBitmapApp $NomeBitmap
		if { [file exists $nome1] } {
			set NomeBitmapApp $nome1b
		}
		if { [file exists $nome2] } {
			set NomeBitmapApp $nome2b
		}
		$canvcop create rect $xi $yi $xf $yf 
        	$canvcop create bitmap $xi $yi -bitmap $NomeBitmapApp -anchor nw
	}

	set ytesto [expr $yf+4*$PuntiPerRiga]

   	set xi [posiz:testo:centrato $xtesto $xf_pagina $ragsoc\
          	$pixellartestobig]
	crea:testo:bold $canvcop $xi $ytesto $ragsoc $font_testibig

	set nomefile $Nomfil_debug
	crea:postscript $canvcop $nomefile

 	if { $tipovis==3 } {
		if [winfo exists .topCopert] {
			destroy .topCopert
 		}
 	}

 }
}



###############################################################
#
#	stampa:indice
#
###############################################################
proc stampa:indice { } {

  global indiceincorso flag_riep
  global xi_pagina xf_pagina yi_pagina yf_pagina tipovis Nomfil_debug
  global xi_scrit xf_scrit yi_scrit yf_scrit PuntiPerRiga
  global canvapp dimfogliox dimfoglioy TipoIndice punti_per_mm
  global flag_stampa_pil flag_stampa_pld flag_stampa_msd
  global flag_stampa_cro flag_stampa_cop flag_stampa_ind
  global nomefilesenzaplc cornice distscrtesto pixellargrande
  global Pagina_Moduli Pagina_Riep_Moduli Pagina_CrossR
  global Pagina_ProgIL Pagina_ProgLD NumeroDellaPagina
  global font_enorme font_testibig font_grande tx_docall tx_index
  global tx_ind_in tx_ind_il tx_ind_ld tx_ind_cr tx_ind_rm tx_ind_io
  global fnt12x18 fnt5x10 fnt4x8 fnt5x7 fnt3x4 fnt4x7

 set NumeroDellaPagina 0

 if { $tipovis==3 } {
 	if { $flag_stampa_pil } {
 		incr Pagina_ProgIL
	}
 	if { $flag_stampa_msd } {
 		incr Pagina_Moduli
 		incr Pagina_Riep_Moduli
	}
 	if { $flag_stampa_cro } {
 		incr Pagina_CrossR
	}
 	if { $flag_stampa_pld } {
 		incr Pagina_ProgLD
	}
 }

 if { $tipovis==1 } {

	coord:pagina $dimfogliox $dimfoglioy
	posiz:titolo:finepag

 	set nupa $NumeroDellaPagina
 	set tipoStp $TipoIndice

	Next_Page

	coord:pagina $dimfogliox $dimfoglioy
    	if {$cornice} {
		crea:cornice $canvapp\
			$xi_pagina $yi_pagina $xf_pagina $yf_pagina
	}

	crea:titolo:finepag $canvapp $tipoStp

	set xtesto [expr $xi_pagina+10*$punti_per_mm]
	set xnumpa [expr $xtesto+100*$punti_per_mm]
	set ytesto [expr $yi_pagina+30*$punti_per_mm]
	set testoi "$nomefilesenzaplc: $tx_docall"

 	set font_appo $fnt4x8
 	crea:testo:bold $canvapp $xtesto $ytesto $testoi $font_appo
	set ytesto [expr $ytesto+3*$PuntiPerRiga]

 	set font_appo $fnt4x7
 	crea:testo:bold $canvapp $xtesto $ytesto $tx_index $font_appo
	set ytesto [expr $ytesto+2*$PuntiPerRiga]

 	if { $flag_stampa_pil } {
		crea:testo $canvapp\
			$xtesto $ytesto $tx_ind_il $font_testibig
		crea:testo:bold $canvapp\
			$xnumpa $ytesto $Pagina_ProgIL $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_msd } {
		#
		# scrive n.pagina riepilogo moduli solo se il riepilogo c'e`
		#
		if { $flag_riep } {
			crea:testo $canvapp $xtesto\
				$ytesto $tx_ind_rm $font_testibig
			crea:testo:bold $canvapp $xnumpa\
				$ytesto $Pagina_Riep_Moduli $font_testibig
			set ytesto [expr $ytesto+2*$PuntiPerRiga]
		}
		crea:testo $canvapp\
			$xtesto $ytesto $tx_ind_io $font_testibig
		crea:testo:bold $canvapp\
			$xnumpa $ytesto $Pagina_Moduli $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_cro } {
		crea:testo $canvapp\
			$xtesto $ytesto $tx_ind_cr $font_testibig
		crea:testo:bold $canvapp\
			$xnumpa $ytesto $Pagina_CrossR $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_pld } {
		crea:testo $canvapp\
			$xtesto $ytesto $tx_ind_ld $font_testibig
		crea:testo:bold $canvapp\
			$xnumpa $ytesto $Pagina_ProgLD $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	# if { $flag_stampa_ind } {
	# 	crea:testo $canvapp\
	# 		$xtesto $ytesto $tx_ind_in $font_testibig
	# 	crea:testo:bold $canvapp\
	# 		$xnumpa $ytesto $nupa $font_testibig
	# 	set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	# }

 } else {

 	wm withdraw .
 	toplevel .topIndice
 	wm title .topIndice "STAMPA INDICE"
        wm protocol .topIndice WM_DELETE_WINDOW procx
 	set frameIndice [frame .topIndice.frmind]    ;# conterra' il canvas
 	set canvind [canvas $frameIndice.canvIndice\
			-width $dimfogliox -height $dimfoglioy]

 	set nupa $NumeroDellaPagina
 	set tipoStp $TipoIndice

	coord:pagina $dimfogliox $dimfoglioy
	posiz:titolo:finepag

    	if {$cornice} {
		crea:cornice $canvind\
			$xi_pagina $yi_pagina $xf_pagina $yf_pagina
	}

	crea:titolo:finepag $canvind $tipoStp

	set xtesto [expr $xi_pagina+5*$distscrtesto]
	set xnumpa [expr $xtesto+18*$pixellargrande]

	set ytesto [expr $yi_pagina+4*$PuntiPerRiga]

	set testoi "$nomefilesenzaplc: $tx_docall"

 	set font_appo $font_grande
 	crea:testo:bold $canvind $xtesto $ytesto $testoi $font_appo
	set ytesto [expr $ytesto+4*$PuntiPerRiga]

 	set font_appo $font_grande
 	crea:testo:bold $canvind $xtesto $ytesto $tx_index $font_appo
	set ytesto [expr $ytesto+2*$PuntiPerRiga]

 	if { $flag_stampa_pil } {
		crea:testo $canvind\
			$xtesto $ytesto $tx_ind_il $font_testibig
		crea:testo:bold $canvind\
			$xnumpa $ytesto $Pagina_ProgIL $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_pld } {
		crea:testo $canvind\
			$xtesto $ytesto $tx_ind_ld $font_testibig
		crea:testo:bold $canvind\
			$xnumpa $ytesto $Pagina_ProgLD $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_msd } {
		crea:testo $canvind\
			$xtesto $ytesto $tx_ind_rm $font_testibig
		crea:testo:bold $canvind\
			$xnumpa $ytesto $Pagina_Riep_Moduli $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
		crea:testo $canvind\
			$xtesto $ytesto $tx_ind_io $font_testibig
		crea:testo:bold $canvind\
			$xnumpa $ytesto $Pagina_Moduli $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

 	if { $flag_stampa_cro } {
		crea:testo $canvind\
			$xtesto $ytesto $tx_ind_cr $font_testibig
		crea:testo:bold $canvind\
			$xnumpa $ytesto $Pagina_CrossR $font_testibig
		set ytesto [expr $ytesto+2*$PuntiPerRiga]
 	}

	set PaginaIndice 1
	set nomefile $Nomfil_debug$PaginaIndice
	crea:postscript $canvind $nomefile

 	if { $tipovis==3 } {
		if [winfo exists .topIndice] {
			destroy .topIndice
 		}
 	}
 }
}



###############################################################
#
#	stampa:ladder:plc - ANCORA DA IMPLEMENTARE
#
###############################################################
proc stampa:ladder:plc { } {
}


###############################################################
#
#	set_tipovis       - SETTA TIPO DI VISUALIZZAZIONE
#
###############################################################
proc set_tipovis { } {

	global	tipovis_appo tipovis

 set tipovis_appo [ConfirmBox -text ConfirmPrintTo]
 set tipovis $tipovis_appo
}



#########################################################################
#
#	INIT:STAMPA:READ:FILE
#
#	OUTPUT:	0 --> tutto okay
#		1 --> errore
#
#########################################################################

proc init:stampa:read:file { t } {

	global	EditPrefs OK_INITIALIZE
	global	indiceincorso tipovis_appo
	global	tipovis VariableList NumeroVariabili
	global	nomvar com_variabile nolines numelin vers_prg
	global	str_pied str_type str_scope str_retain tipdat
	global	str_location str_modulo str_ioline str_initval
	global	nomefileplc tx_niet tx_da data_ulmo tx_print
	global  nomefilesenzaplc

 if { $tipovis==0 } {
	return 1	;# PREMUTO RINUNCIA
 }

 Init:Variabili:Stampa

 if { $tipovis==1 && $OK_INITIALIZE==0 } {
					return 1	;# PREMUTO RINUNCIA
 }

 set nomefileplc [file tail [GetFilename $t]]
 #set lung [string length $nomefileplc]
 #incr lung -5
 #set nomefilesenzaplc [string range $nomefileplc 0 $lung]
 set nomefilesenzaplc [file rootname $nomefileplc]

 #
 # PREPARA TUTTI I DATI DA VISUALIZZARE OPPURE STAMPARE
 #

 set indexx_n 0
 set vers_prg "-"

 if { [array get VariableList]=="" } {
	return 1
 } else {
     foreach i [lsort -ascii [array names VariableList]] {

	set nomvar($indexx_n) $i
	set com_variabile($indexx_n) [lindex $VariableList($i) 5]
	set str_location($indexx_n) [lindex $VariableList($i) 3]
	set str_scope($indexx_n) [lindex $VariableList($i) 2]
	set str_type($indexx_n) [lindex $VariableList($i) 1]
	set str_initval($indexx_n) [lindex $VariableList($i) 4]
	set app1	[lindex $VariableList($i) 0]
       	set app2	[string range $app1 0 1]

	if { [string compare $nomvar($indexx_n) "_VERSION"]==0 } {
		set vers_prg $str_initval($indexx_n)
	}

	if { [string compare $nomvar($indexx_n) "_CREATED"]==0 } {
		set data_ultima_mod $str_initval($indexx_n)
 		set pday  [clock format $data_ultima_mod -format %d]
 		set pmon  [clock format $data_ultima_mod -format %m]
 		set pyear [clock format $data_ultima_mod -format %y]
 		if { $tipdat=="g" } {
 			set data_ulmo "$pday/$pmon/$pyear" ;# EUROPEAN DATE
 		} else {
 			set data_ulmo "$pmon/$pday/$pyear" ;# AMERICAN DATE
 		}
	}

	if { [string tolower $app2] == "no" } {
		set str_retain($indexx_n) $tx_niet
	} else {
		set str_retain($indexx_n) $tx_da
	}

	if { [string tolower $str_location($indexx_n)] == "memory" } {
		set str_pied($indexx_n)   "-"
		set str_modulo($indexx_n) "-"
		set str_ioline($indexx_n) "-"
	} else {
		set str_pied($indexx_n)   [lindex $VariableList($i) 11]
		set str_modulo($indexx_n) [lindex $VariableList($i) 10]
		set str_ioline($indexx_n) [lindex $VariableList($i) 9]
	}

	set indexx_n2 0
	set ind "1.0"
	set first ""
	set finericerca 0

	while {$finericerca==0} {
    		set ind [$t search -exact $i $ind]
    		if {$ind==$first} {
       			set finericerca 1
    		} else {
       			if {$first=={} } {
          			set first $ind
       			}
       			set linea [string range $ind 0 [expr\
						[string first "." $ind]-1]]
       			if {[$t get "$ind wordstart" "$ind wordend"]==$i} {
	   			set numelin($indexx_n,$indexx_n2) $linea
	   			incr indexx_n2
       			}
       			set ind "[incr linea].0"
    		}
	} ;#endwhile

	set nolines($indexx_n) $indexx_n2
	incr indexx_n

     } ;# END FOREACH

     set NumeroVariabili $indexx_n

     return 0
 }
}


##################################################################
#
# setta:formati:stampa	setta le variabili per la gestione
#			modifica dimensioni pagina
#
##################################################################
proc setta:formati:stampa {} {

  global tipi_foglio larghezza_foglio altezza_foglio
  global ima_labels gallo_labels opt_labels altez_labels largh_labels
  global Home

 set tipi_foglio { "Letter" "Legal" "Executive" "Universal" "Tabloid" \
                  "Full11x17" "12x19" "A3" "A4" "A5" "B4" "B5" "B6" "Custom" }

 set ima_labels(Letter)		"$Home/icone/a4.png"
 set ima_labels(Legal)		"$Home/icone/legal.png"
 set ima_labels(Executive)	"$Home/icone/a5.png"
 set ima_labels(Universal)	"$Home/icone/univer.png"
 set ima_labels(Tabloid)	"$Home/icone/legal.png"
 set ima_labels(Full11x17)	"$Home/icone/univer.png"
 set ima_labels(12x19)		"$Home/icone/univer.png"
 set ima_labels(A3)		"$Home/icone/a3.png"
 set ima_labels(A4)		"$Home/icone/a4.png"
 set ima_labels(A5)		"$Home/icone/a5.png"
 set ima_labels(B4)		"$Home/icone/a4.png"
 set ima_labels(B5)		"$Home/icone/a5.png"
 set ima_labels(B6)		"$Home/icone/univer.png"
 set ima_labels(Custom)		"$Home/icone/person.png"
 
 set gallo_labels(Letter)	"Letter"
 set gallo_labels(Legal)	"Legal"
 set gallo_labels(Executive)	"Executive"
 set gallo_labels(Universal)	"Universal"
 set gallo_labels(Tabloid)	"Tabloid"
 set gallo_labels(Full11x17)	"Full11x17"
 set gallo_labels(12x19)	"12x19"
 set gallo_labels(A3)		"A3"
 set gallo_labels(A4)		"A4"
 set gallo_labels(A5)		"A5"
 set gallo_labels(B4)		"B4"
 set gallo_labels(B5)		"B5"
 set gallo_labels(B6)		"B6"
 set gallo_labels(Custom)	"Custom"
 
 set altez_labels(Letter)	"279.4"
 set altez_labels(Legal)	"355.6"
 set altez_labels(Executive)	"266.7"
 set altez_labels(Universal)	"431.8"
 set altez_labels(Tabloid)	"431.8"
 set altez_labels(Full11x17)	"442.0"
 set altez_labels(12x19)	"495.3"
 set altez_labels(A3)		"420.2"
 set altez_labels(A4)		"297.0"
 set altez_labels(A5)		"210.3"
 set altez_labels(B4)		"257.2"
 set altez_labels(B5)		"257.2"
 set altez_labels(B6)		"182.0"
 set altez_labels(Custom)	""
 
 set largh_labels(Letter)	"215.9"
 set largh_labels(Legal)	"215.9"
 set largh_labels(Executive)	"184.2"
 set largh_labels(Universal)	"297.0"
 set largh_labels(Tabloid)	"279.4"
 set largh_labels(Full11x17)	"297.4"
 set largh_labels(12x19)	"297.4"
 set largh_labels(A3)		"297.0"
 set largh_labels(A4)		"209.9"
 set largh_labels(A5)		"148.2"
 set largh_labels(B4)		"363.7"
 set largh_labels(B5)		"182.0"
 set largh_labels(B6)		"128.4"
 set largh_labels(Custom)	""
 
 set opt_labels(Letter)		"Letter   "
 set opt_labels(Legal)		"Legal    "
 set opt_labels(Executive)	"Executive"
 set opt_labels(Universal)	"Universal"
 set opt_labels(Tabloid)	"Tabloid  "
 set opt_labels(Full11x17)	"Full11x17"
 set opt_labels(12x19)		"12x19    "
 set opt_labels(A3)		"A3       "
 set opt_labels(A4)		"A4       "
 set opt_labels(A5)		"A5       "
 set opt_labels(B4)		"B4       "
 set opt_labels(B5)		"B5       "
 set opt_labels(B6)		"B6       "
 set opt_labels(Custom)		"Custom   "
 
}


##################################################################
#
# gestisce_tixcontrol	abilita o disabilita la modifica
#			delle dimensioni pagina
#
##################################################################
proc gestisce_tixcontrol { app } {

  global largh_labels altez_labels

 set forma $app
 set dim1  $altez_labels($app)
 set dim2  $largh_labels($app)
 if { $forma=="Custom" } {
	[.topopz.prtopz subwidget frm].fr2.altez configure -state normal
	[.topopz.prtopz subwidget frm].fr2.largh configure -state normal
	[.topopz.prtopz subwidget frm].fr2.lab_altez\
				configure -foreground black
	[.topopz.prtopz subwidget frm].fr2.lab_largh\
				configure -foreground black
	[.topopz.prtopz subwidget frm].fr3.lgpag\
				configure -foreground black
	[.topopz.prtopz subwidget frm].fr4.lgpag\
				configure -text ""
 } else {
	[.topopz.prtopz subwidget frm].fr2.altez configure -state disabled
	[.topopz.prtopz subwidget frm].fr2.largh configure -state disabled
	[.topopz.prtopz subwidget frm].fr2.lab_altez\
				configure -foreground gray50
	[.topopz.prtopz subwidget frm].fr2.lab_largh\
				configure -foreground gray50
	[.topopz.prtopz subwidget frm].fr3.lgpag\
				configure -foreground gray50
	[.topopz.prtopz subwidget frm].fr4.lgpag\
				configure -text "$dim1 x $dim2"
 }
}



####################################################################
#
# Procedura TestVarConnesse
# =========================
#
# Procedura che estrae dal vettore delle variabili quelle che sono
# connesse con i moduli
#
# Input: 
#   mod = stringa 3 lettere nome modulo (ios,mpp,udc,ecc....)
#   numinp = numero input del modulo
#   numout = numero output del modulo
#
# Ritorna :
#   1 = modulo utilizzato
#   0 = modulo non utilizzato
#
####################################################################
proc TestVarConnesse { tipomod numinp numout } {

 global VariableList linea_udc linea_mpp

 set modulousato 0

#
# per ora max_num_linee e` settato bovinamente a 4
#

 for { set lin 0 } { $lin < 4 } { incr lin } {

  if { $tipomod=="udc" } {
  	set linea_udc($lin) 0
  }
  if { $tipomod=="mpp" } {
  	set linea_mpp($lin) 0
  }

  foreach i [array names VariableList] {
    if { [lindex $VariableList($i) 8] != {} && \
      [lindex $VariableList($i) 1] == "BOOL" } {
        if { [string first [string toupper $tipomod] \
             [string toupper [lindex $VariableList($i) 8]]] != -1} {
          set ElementiConnessione [split [lindex $VariableList($i) 8] "."]
          if { [lindex $ElementiConnessione 1] == $lin} {
		set modulousato 1
  		if { $tipomod=="udc" } {
  			set linea_udc($lin) 1
  		}
  		if { $tipomod=="mpp" } {
  			set linea_mpp($lin) 1
		}
          }
        } ;# endif
      } ;# endif
  } ;# end foreach
 } ;# end for

 return $modulousato
} ;# end proc



###########################################################################
#
# cerca:io:speciali
#
# - cerca le variabili di I/O non connesse a un dispositivo IOS e
#	chiama la procedura "pagina:variab:nonios" per creare la
#	pagina relativa
#
###########################################################################

proc cerca:io:speciali { canv numpa } {

 global VariableList variabiliNonIos

 set variabiliNonIos 0

 foreach nomevar [lsort -ascii [array names VariableList]] {

	set p8		[lindex $VariableList($nomevar) 8 ]
	set appoggio	[string toupper $p8]
	set appoggio1	[split $appoggio "."]
	set tiptip	[lindex $appoggio1 0]

	if { $p8 != "" && $appoggio != "NULL" && $appoggio != "IOS" } {
		incr variabiliNonIos
	}
 }

 if { $variabiliNonIos != 0 } {
 	pagina:variab:nonios $canv $numpa
 }
}


##################################################
##						##
## PAGINA:VARIAB:NONIOS				##
## --------------------				##
##						##
##################################################

proc pagina:variab:nonios { canv num_pagina } {

	global	pixelalttestobig pixelalttesto pixelaltgrande
	global	indiceincorso VariableList
	global	SpesMed pixellartestobig fnt3x4 font_testi
	global	cornice TipoTabIO dist_testilinee punti_per_mm
	global	pixelaltgrande pixellargrande font_grande distscrtesto 
	global	pixelaltenorme titolo_riep pixellarenorme font_testibig
	global	font_enorme tipovis xi_scrit xf_scrit font_granden font_titfp
	global	dimfoglioy numline_riep y_titolofp frameModuli max_num_linee 
	global	dimfogliox xi_pagina xf_pagina yi_pagina yf_pagina
	global  SaltaFormFeed tipovis dx_pagina dy_pagina
	global	NumeroDellaPagina Nomfil_debug PuntiPerRiga

 set tipoStp $TipoTabIO		;# Titolo di fine pagina
 set filestampa $Nomfil_debug

#
# LOOP PRODUZIONE PAGINA DI "RIEPILOGO MODULI"
#
  
 set conta_lin 0
#
# prima di ogni cosa, svuoto completamente il canvas
#
 if { $tipovis==1 } {
	Next_Page
 } else {
	if [info exist frameModuli] {
		$canv delete tagMod
	}
 }

 if {$cornice} {
	crea:cornice $canv $xi_pagina $yi_pagina $xf_pagina $yf_pagina 
 }

#
# scrittura intestazione pagina
#
 if { $tipovis==1 } {
	set dimensione_intestazioni2 [expr $punti_per_mm*6]
	set dimensione_intestazioni [expr $punti_per_mm*9]
 } else {
	set dimensione_intestazioni2 [expr $punti_per_mm*5]
	set dimensione_intestazioni [expr $punti_per_mm*7]
 }

 set posy [expr $yi_pagina+$dimensione_intestazioni ]

 if { $tipovis==1 } {

	crea:testo:centrato:bold $xi_pagina $xf_pagina\
				[expr $yi_pagina+2*$dist_testilinee]\
					[GetString titolo_nonios] $font_enorme
	if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}
 } else {

	$canv create line $xi_pagina $posy $xf_pagina $posy
	set xi [posiz:testo:centrato $xi_pagina $xf_pagina\
					[GetString titolo_nonios] $pixellartestobig]
   	crea:testo:bold $canv $xi [expr $posy-$dimensione_intestazioni/2]\
					[GetString titolo_nonios] $font_testibig
   	set posy [expr $posy+$dimensione_intestazioni]
   	$canv create line $xi_pagina $posy $xf_pagina $posy
 }

#
# LINEA DI SEPARAZIONE INTESTAZIONE
#
 if { $tipovis==1 } {
	if { $indiceincorso==1 } {
		PrtLine -S$SpesMed -P$xi_pagina,$posy,$xf_pagina,$posy
	}
 } else {
	$canv create line $xi_pagina $posy $xf_pagina $posy
 }

#
# SE NO CORNICE, CREA CONTORNO INTESTAZIONE
#
 if {$cornice=="false"} {
	if { $tipovis==1 } {
		if { $indiceincorso==1 } {
			PrtLine -S$SpesMed\
				-P$xi_pagina,$yi_pagina,$xf_pagina,$yi_pagina
			PrtLine -S$SpesMed\
				-P$xi_pagina,$yi_pagina,$xi_pagina,$posy
			PrtLine -S$SpesMed\
				-P$xf_pagina,$yi_pagina,$xf_pagina,$posy
		}
	} else {
		$canv create line $xi_pagina $yi_pagina $xf_pagina $yi_pagina
		$canv create line $xi_pagina $yi_pagina $xi_pagina $posy
		$canv create line $xf_pagina $yi_pagina $xf_pagina $posy
      	}
 }

#
# LOOP PER DISEGNO DEI BLOCCHI PER DESCRIZIONE LINEA 
#
 set xic [expr $xi_pagina + 10*$punti_per_mm]
 set yapp [expr $posy + 10*$punti_per_mm]

 if { $tipovis==1 } {
	set deltay $pixelaltgrande
	set font_appo $fnt3x4
 } else {
	set deltay $pixelalttestobig
	set font_appo $font_testibig
 }

 incr deltay $punti_per_mm
 incr deltay $punti_per_mm

 set numero_var 0

 foreach nomevar [lsort -ascii [array names VariableList]] {

    set parte8     [lindex $VariableList($nomevar) 8 ]
    set parte8maiu [string toupper $parte8]

    if { $parte8 != "" && $parte8maiu != "NULL" && $parte8maiu != "IOS" } {

	set parte8campo1 [split $parte8maiu "."]
	set parte8tipo   [lindex $parte8campo1 0]
	set ce_mpp  [string first "MPP" $parte8maiu]
	set ce_udc  [string first "UDC" $parte8maiu]

	if { $parte8tipo != "IOS" || ( $ce_mpp==-1 && $ce_udc==-1 )} {

		set p1  [lindex $VariableList($nomevar) 1 ]
		set p5  [lindex $VariableList($nomevar) 5 ]

		incr numero_var

		set xic0 [expr $xic + 15*$punti_per_mm]
		set xic1 [expr $xic + 40*$punti_per_mm]
		set xic2 [expr $xic + 70*$punti_per_mm]

		if { $numero_var < 10 } {
			set testo "$numero_var  :"
		} else {
			set testo "$numero_var :"
		}
		crea:testo:bold $canv $xic $yapp $testo $font_appo

		set testo "$p1"
		crea:testo:bold $canv $xic0 $yapp $testo $font_appo

		set testo "$nomevar"
		crea:testo:bold $canv $xic1 $yapp $testo $font_appo

		set testo "$parte8"
		crea:testo:bold $canv $xic2 $yapp $testo $font_appo

		if { $p5 != "" } {
 			incr yapp $deltay
			set testo "$p5"
			crea:testo:bold $canv\
				$xic1 $yapp $testo $font_appo
 			incr yapp $deltay
		} else {
 			incr yapp $deltay
		}
	}
    } ;# end-if
 } ;# end-foreach

 crea:titolo:finepag $canv $tipoStp

 if { $tipovis!=1 } {
	$canv addtag tagMod all

	if { $tipovis==3 } {
		set nomefile "$filestampa$NumeroDellaPagina" 
		crea:postscript $frameModuli.canvMod $nomefile
	}
 }

 set numero_pagine_scritte 1

 return $numero_pagine_scritte
}

