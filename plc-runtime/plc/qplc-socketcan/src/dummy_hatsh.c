/*
* @(#) dummy_hatsh.c 1.2 Fri Jul 28 17:00:05 MET DST 2000
*
* Pseudo-hatsh : contiene un "falso" hatsh, che , chiamato, provoca un errore
* fatale.
*
* 20/05/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta la primitiva "esci".
* 28/07/00 GG 1.2 Aggiustamenti per compilazione su Linux.
*/

char ver_hatsh[] = "";

int iHShOk = 0;
void * cpaxShm = (void *) 0;

int CnInit(void)
{
#ifdef Linux
	return 1;
#else
/* ERRORE. Modulo hat-sh non caricato. */
	plcError(199,"hatsh.oo");
	exit(0);
	return 0;
#endif
}

void hat(void)
{
}

void shoes(int s)
{
}

int hshEnable(void)
{
	iHShOk = 0;
	return 0;
}

void esci(void)
{
}
