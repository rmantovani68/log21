/*
* @(#) mmap.h 1.0 Mon May  8 10:58:02 WET 1995
*
* La funzione "mmap" fornisce un metodo trasparente di accesso ai
* dispositivi mappati in memoria. Il driver del dispositivo deve
* essere in grado di fornire il servizio "IOCTL_MMAP".
* La "mmap" qui realizzata e` formalmente uguale alla system call omonima
* di SystemVr4, ma funziona solo se "addr", ed "off" sono nulli e "len"
* e` nullo o uguale alla dimensione caratteristica dell'oggetto mappato.
*/

#ifndef _MMAP_H

/* Flag di abilitazione dei vari tipi di accesso ad un'area condivisa
 (parametro "prot" della funzione "mmap" e campo "prot" di "mmap_t"). */

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4		/* Non supportato dai processori i80x86. */
#define PROT_NONE 0

/* Flag di abilitazione della modalita` di creazione del segmento di
 shared memory. */

#define MAP_SHARED 1	/* Shared memory ordinaria.
							E` L'UNICA MODALITA` AMMESSA DA LYNX. */
#define MAP_PRIVATE 2	/* Crea un'immagine privata dell'area. 
							NON AMMESSO SU LYNX. */
#define MAP_FIXED 4		/* Usa l'indirizzo indicato dall'utente.
							NON AMMESSO SU LYNX. */

/* Codice del servizio che tutti i dispositivi "mmap-abili" devono
 fornire. Quest'ioctl interpreta il terzo argomento come puntatore ad
 una una struttura di tipo "mmap_t", che viene riempita con i dati
 specifici del dispositivo. */
 
#define IOCTL_MMAP 0xeeff

/* Massima lunghezza di un nome di segmento condiviso. */

#define MAXMMAPNAME 31

/* Struttura descrivente il segmento di memoria condivisa associato
 ad un dispositivo. Le informazioni contenute in essa sono in genere
 fornite dal servizio "IOCTL_MMAP" del driver del dispositivo. */

struct mmap_t {
	unsigned int phaddr;	/* Indirizzo fisico del dispositivo. */
	unsigned int size;		/* Dimensione dell'area ricoperta. */
	int prot;				/* Tipi di accesso consentiti. */
	char name[MAXMMAPNAME+1];	/* Nome del segmento di shared memory. */
};

/*
* Funzione "mmap"
* ---------------
*
*  Questa funzione realizza un'interfaccia verso le primitive di accesso
* alla memoria fisica associata ad un dispositivo. Il dispositivo e`
* rappresentato dal file descriptor "fildes", e deve supportare il servizio
* IOCTL_MMAP. I parametri "addr", "off" e "len" sono presenti solo per
* mantenere la compatibilita` formale con SystemVr4, dove "mmap" e` una
* system call, e possono essere posti a 0. Il parametro "prot" specifica
* il tipo di accesso all'area condivisa, e deve contenere l'OR bit a bit
* dei valori "PROT_READ", "PROT_WRITE" e "PROT_EXEC". Il parametro "flags"
* specifica alcuni attributi aggiuntivi per l'area mappata. Su LynxOS,
* l'unico valore ammesso e` "MAP_SHARED".
*
*  Il valore di ritorno della funzione e` il puntatore all'area condivisa,
* oppure (char *) -1 in caso di errore.
*/


char * mmap(char * addr, unsigned int len, int prot, int flags, int fildes, unsigned int off);
/*
* Funzione "munmap"
* -----------------
*
*  Questa funzione sgancia lo spazio virtuale del processo chiamante
* dall'area condivisa "addr" ottenuta con "mmap". La funzione vale sempre
* 0, perche` le primitive utilizzate non segnalano errori se "addr" non
* contiene un valore accettabile.
*  Il parametro "len" e` ignorato.
*/

int munmap(char * addr, unsigned int len);

#define _MMAP_H

#endif


