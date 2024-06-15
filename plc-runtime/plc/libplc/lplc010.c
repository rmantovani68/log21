/* lplc010.c */
#include "lplc.h"

/*
* Funzione ReadIsaConf
* --------------------
*
* Legge il file di configurazione "name", se esiste.
*
*  Vale 1 in caso di successo, 0 se il file non esiste.
*
*  Il file puo` contenere commenti nella forma tipica di Unix (# comento).
*
* NOTA : Questa versione ridotta della funzione. Non e` eseguito alcun
* controllo sui dati, e sono semplicemente ignorate le opzioni sconosciute.
*/

static int ReadIsaConf(char *name)
{
FILE * stream;
char line[MAX_CNF_LINE+2],filler,c;
long l;
int i,ln,n;

/* Tenta di aprire il file. */

	stream = fopen(name,"r");

/* Se il file esiste, tenta di leggere i parametri. */

	if (stream) {
		ln = 0;
		for (;;) {
			if (! fgets(line,sizeof(line)-1,stream))
				break;
			++ln;
			if (sscanf(line," SEMMASK = %o",& iSemPerms) == 1) {
				continue;
			}
			if (sscanf(line," MEMMASK = %o",& iMemPerms) == 1) {
				continue;
			}
			if (sscanf(line," MEMSIZE = 0x%lx",& lMemSize) == 1 ||
			    sscanf(line," MEMSIZE = %ld",& lMemSize) == 1) {
				continue;
			}
			if (sscanf(line," SEMKEY = %ld",& l) == 1) {
				iSemKey = (int) l;
				continue;
			}
			if (sscanf(line," MEMKEY = %ld",& l) == 1) {
				iMemKey = (int) l;
				continue;
			}
		}
		fclose(stream);
		return 1;
	}
	return 0;
}

/*
* Funzione plcTryAttach(flag)
* -----------------------
*
* Questa funzione collega lo spazio condiviso del PLC al processo chiamante.
* Se "flag" vale 1, il processo chiamante e` bloccato finche` il Plc non ha
* allocato tutte le risorse di sistema necessarie.
* Se "flag" vale 0 ed il Plc non e` ancora partito, la funzione esce
* immediatamente con errore.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcTryAttach(int flag)
{
int rv;

	/* Lettura della configurazione. */

	rv = ReadIsaConf(szConfigFile);
	if (rv < 0) {
		fprintf(stderr,
			"plcInit: syntax error in configuration file \"%s\" at line %d\n",
			szConfigFile, -rv);
		return -1;
	}

/* Attende la creazione del semaforo. */

	do {
		iSemId = semget(iSemKey, MAX_SEM, iSemPerms);
		if (iSemId < 0)  {
			if (errno != ENOENT) {
				perror("semget()");
				return -1;
			}
			else if (! flag)
				return -1;
			sleep(SMEM_POLL_TIME);
		}
	} while (iSemId < 0);

/* Attende la creazione dell'area condivisa. */
	
	do {
		iMemId = shmget(iMemKey, lMemSize, iMemPerms);
		if (iMemId < 0)  {
			if (errno != ENOENT) {
				perror("shmget()");
				return -1;
			}
			else if (! flag)
				return -1;
			sleep(SMEM_POLL_TIME);
		}
	} while (iMemId < 0);

/* Acquisizione dell'area condivisa. */

/* Qui la semaforizzazione serve solo per assicurare la sincronizzazione
 con il creatore della risorsa. */

	plcPSem(SEM_MEM);

	pchMem = (char *) shmat(iMemId, (char *)0, 0);
	if (pchMem == (char *) -1) {
		perror("shmat()");
		plcVSem(SEM_MEM);
		return -1;
	}

	/* Calcola l'indirizzo del direttorio degli spazi e
	 dei blocchi liberi. */

	spDir = (struct spc_t *) pchMem;
	spFree = spDir + MAX_TOTSPACE;
	pStatus = (long *)(spFree + MAX_TOTSPACE);

	plcVSem(SEM_MEM);

	return 0;
}


/*
* Funzione plcAttach()
* --------------------
*
* Questa funzione collega lo spazio condiviso del PLC al processo chiamante.
* Il processo chiamante e` bloccato finche` il processo plc non ha allocato
* tutte le risorse di sistema necessarie.
* Vale 0 in caso di successo, -1 in caso di errore.
*/

int plcAttach(void)
{
	return plcTryAttach(1);
}

