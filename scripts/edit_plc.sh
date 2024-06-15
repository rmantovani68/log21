#!/bin/bash
#
# Lanciatore di "edit_plc" per ambienti non-XNC (es. Linux).
# Questo file dovrebbe essere rinominato "edit_plc", ed essere raggiungibile
# dal PATH di default dei processi. Un posto adatto ad ospitarlo e` di
# solito /usr/local/bin. Il pacchetto "edit_plc" deve invece risiedere in
# una directory normalmente inaccessibile, per esempio /usr/local/lib/edit_plc
# o, piu` tradizionalmente, /home/edit_plc.

#
# Funzione per la creazione del file di configurazione iniziale.
#
make_cnf()
{
	echo '
*** Attenzione! '"$cnf"' non presente.
*** Verra` creata una versione ragionevole della configurazione di default
*** in questa directory.'
	sed -e '
s:^set EditPrefs(binpath).*$:set EditPrefs(binpath) '"$EDITPLC_PROJECT"':
s:^set EditPrefs(binpathremoto).*$:set EditPrefs(binpathremoto) '"$EDITPLC_PROJECT"':
s:^set EditPrefs(objpath).*$:set EditPrefs(objpath) '"$EDITPLC_PROJECT"'/plc-objs:
s:^set EditPrefs(objpathlocale).*$:set EditPrefs(objpathlocale) '"$EDITPLC_PROJECT"'/plc-objs:
s:^set EditPrefs(objpathremoto).*$:set EditPrefs(objpathremoto) '"$EDITPLC_PROJECT"'/plc-objs:
s:^set EditPrefs(progpath).*$:set EditPrefs(progpath) '"$EDITPLC_PROJECT"':
s:^set EditPrefs(progpath1).*$:set EditPrefs(progpath1) '"$EDITPLC_PROJECT"':
s:^set EditPrefs(progpath1locale).*$:set EditPrefs(progpath1locale) '"$EDITPLC_PROJECT"':
s:^set EditPrefs(progpath1remoto).*$:set EditPrefs(progpath1remoto) '"$EDITPLC_PROJECT"':' $EDITPLC_HOME/defaults.tcl.$os > $cnf
}


#
# Funzione per la creazione di un file "scambio" minimo.
#

make_scambio()
{
	echo '
*** Attenzione! File "scambio" generato automaticamente
*** per la configurazione "qplc stand-alone".
*** Per le configurazioni "XNC standard" e "XNC multicentro", copiare
*** manualmente uno dei file '"$EDITPLC_HOME"'/util/scambio*,
*** oppure cancellare il file generato ora. '
	echo '
(* Segnali di scambio statici predefiniti (modalita` "-shvmask 0" di qplc *)

VAR_GLOBAL
    CNOK                     AT %MX11     : BOOL ;
END_VAR
' > scambio
}

#
# Funzione per la costruzione di un file "scambdin" minimo.
#

make_scambdin()
{
	echo '
*** Attenzione! Sara` generato automaticamente un file "scambdin"
*** vuoto.
*** Per le configurazioni "XNC standard" e "XNC multicentro", copiare
*** manualmente uno dei file '"$EDITPLC_HOME"'/util/scambdin*,
*** oppure cancellare il file generato ora. '
		echo '
(* Segnali di scambio dinamici (progetto '"$EDITPLC_PROJECT"') *)
' > scambdin
}

#
# Identificazione del sistema operativo.
#

os=`uname`

#
# Configurazioni globali del pacchetto.
#

# Qui deve risiedere il pacchetto edit_plc.
export EDITPLC_HOME=/usr/local/cni/edit_plc
# Configurazione di default
#export EDITPLC_HOME=/home/edit_plc
export EDITPLC_PROJECT=`pwd`
# Configurazione di default
#export EDITPLC_PROJECT=/home/edit_plc
export EDITPLC_CONF=edit_plc.conf
# Configurazione di default
#export EDITPLC_CONF=defaults.tcl

#
# Se il file di configurazione non e` presente,
# ne viene creato uno ragionevole.
# Vengono anche creati i file "scambio" e "scambdin" minimi.
#

cnf=$EDITPLC_PROJECT/$EDITPLC_CONF
if [ -f $cnf ]; then
	:
else
	make_cnf
	if [ -f scambio ]; then
		:
	else
		make_scambio
	fi
	if [ -f scambdin ]; then
		:
	else
		make_scambdin
	fi
fi

#
# Costruzione del path di lavoro
# del pacchetto edit_plc.
#

export PATH=$EDITPLC_HOME:$EDITPLC_HOME/bin.$os:$PATH

#
# Lancio dell'editor.
#

cd $EDITPLC_HOME && exec ./edit_plc

