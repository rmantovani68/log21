/*
* @(#) asend.c 1.0 Mon Aug 26 12:06:54 MET DST 1996
*
*  Questo file contiene le primitive di colloquio asincrono con
* lo smistatore. L'invio del messaggio e` affidato ad un thread separato,
* controllato da un semaforo.
*/

pthread_t util_start_thread(void *(*f)(void *), int prio);
int util_execprio(void);

typedef struct {
	int type;
	long data[4];
} xnc_msg_t;

static struct {
	pthread_t tid;
	csem_t sem;
	int qfirst;
	int qlast;
	xnc_msg_t d[MAX_ASEND_QUEUE];
} asend_status;

/*
* Funzione "asend_enqueue"
* ------------------------
*
*  Questa funzione accoda un messaggio (16 byte) di un dato tipo. Il
* tipo individua la componente del CN a cui e` destinato.
*/

void asend_enqueue(int type, long *data)
{
int n;

	n = asend_status.qlast;
	asend_status.d[n].type = type;
	memcpy(asend_status.d[n].data, data, sizeof(asend_status.d[n].data));
	asend_status.qlast = (n + 1) % MAX_ASEND_QUEUE;

	csem_signal(asend_status.sem);
}

/*
* Questo e` il "cuore" del supervisore. Controlla i canali di
* comunicazione col debugger e l'attivita` del supervisore.
*/

static void * asend_main(void * arg)
{
int n;

	for (;;) {

	/* Attesa sul semaforo. */

		csem_wait(asend_status.sem, (struct timeval *) 0);

	/* Elaborazione del messaggio. */

		n = asend_status.qfirst;

		switch (asend_status.d[n].type) {
		case ASEND_GMM:
			teccy_socket(sock,(int *)asend_status.d[n].data);
			break;
		case ASEND_AX:
           		if(sendto(Dati_Assi.sveglia,(char *)asend_status.d[n].data,4*8,NULL,\
                		(struct sockaddr *)(&Dati_Assi.soap),sizeof(Dati_Assi.soap))<0)
               				invXerror("HAX",1,"%d%v",errno);         
 
			break;
		default:
			break;
		}

		asend_status.qfirst = (n + 1) % MAX_ASEND_QUEUE;
	}
}

/*
* Funzione "asend_start"
* -----------------------
*
*  Questa funzione lancia il thread supervisore.
*/

void asend_start(void)
{
/* Inizializzazione della struttura di stato del supervisore. */

/* Creazione del semaforo di attivazione del supervisore. */

	asend_status.sem = csem_create_val(0);

	asend_status.qfirst = 0;
	asend_status.qlast = 0;

/* Lancio del thread. */

	asend_status.tid = util_start_thread(asend_main, util_execprio() - 1);
}

