/*
* @(#) dummy_hatsh.c 1.1 Fri Aug 30 16:54:59 MET DST 1996
*
* Pseudo-hatsh : contiene un "falso" hatsh, che , chiamato, provoca un errore
* fatale.
*
* 20/05/96 GG 1.0 Prima stesura.
* 30/08/96 GG 1.1 Aggiunta la primitiva "esci".
*/

char ver_hatsh[] = "";

int iHShOk = 0;
void * cpaxShm = (void *) 0;

int CnInit(void)
{
/* ERRORE. Modulo hat-sh non caricato. */
	return 1;
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
