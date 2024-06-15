
#ifndef _IOSCNI_H

#define _IOSCNI_H

/* Numero massimo di controllori per IOS (ottimistico). */

#define MAX_IOS_BOARD 4 /* Attenzione! Informazione
						   duplicata (ioscni_priv.h) */
#define MAX_IOS_CPU 8

/* Struttura di stato di un'istanza del dispositivo. */

struct iosboard {

/* IO base address del dispositivo. */

	unsigned int ioaddr;

/* Indirizzo fisico in memoria del dispositivo. */

	unsigned long memaddr;

/* Offset e dimensione dei controller (size = 0 : assente). */

	int cpu_off[MAX_IOS_CPU];
	int cpu_size[MAX_IOS_CPU];

/* Offset e dimensione della RAM non volatile (size = 0 : assente). */

	int nvram_off;
	int nvram_size;

/* Dimensione dell'E2 seriale (0 : assente). */

	int e2_size;
};

/* Struttura descrivente una porta. */
struct ios_port_descr_t {
/* Indirizzo della porta. */
int ioaddr;
/* Numero di bit. */
int nbits;
/* Posizione del primo bit. */
int pos;
};

/* Struttura principale, utilizzata da IOSGETPORTS. */
struct ios_ports_t {
/* Tipo di insieme di porte scelto. */
int port_set;
/* Numero di posti nell'array "ports". */
int nports;
/* Area dove scaricare i dati (variabile). */
struct ios_port_descr_t ports[1];
};

/* Codici IOCTL */
#define IOSIOCTL ('I' << 8)

/* Copia nella struttura iosboard lo stato corrente del dispositivo. */
#define IOSGET (IOSIOCTL + 1)


#define IOS_MIN_BRD_SZ 0x400 /* Dimensione minima di un controller. */
#define IOS_MAX_BRD_SZ 0x1000 /* Dimensione massima di un controller. */
#define IOS_NVRAM_SZ 0x2000	/* dimensione RAM non volatile. */

#define IOS_SHMSIZE 0x4000	/* Dimensione totale dell'area condivisa. */

/*
* Descrizione dei byte di un controllore.
*/

/* Immagine dei moduli (8 byte per modulo). */
#define IOS_IODATA 0x000
/* Copie delle immagini. */
#define IOS_IOCOPY 0x100
/* Byte di stato (tabella di 32 elementi). */
#define IOS_IOSTATUS 0x200
/* Indirizzo del modulo da rinfrescare immediatamente. */
#define IOS_IMMEDIATE 0x220
/* Flag di modulo remoto sganciato. Deve essere portato a 0, passa a 0x55
 quando un modulo si sgancia. */
#define IOS_DISCONN 0x221
/* Flag di "controller OK". */
#define IOS_CPUOK 0x222
/* Statistica errori (2 byte). */
#define IOS_ERRCOUNT 0x223
/* Offset del campo versione firmware (8 byte). */
#define IOS_VER 0x225
/* Errori per modulo (tabella di 32 elementi). */
#define IOS_ERRORS 0x22D
/* Flag di (non) riaggancio automatico dei moduli remoti. */
#define IOS_HANDSHAKE 0x24D
/* Semafori per condivisione di un byte tra controllore e
 applicativo (2 byte). */
#define IOS_IOSEM 0x24E
/* Periodo di retrigger. */
#define IOS_WDPRESET 0x259
/* Ingresso di trigger per il watchdog. */
#define IOS_WDTRIG 0x25A
/* Posizione della firma che fa ripartire il controller dopo un timeout. */
#define IOS_PASSWD 0x25B

/* Firma da scrivere in IOS_FIRMA. */
#define IOS_MAGIC { 0x19, 0x04, 0x72, 0x69, }

/* Maschere per l'estrazione dei campi dal byte di descrizione di un modulo
 (tabella IOS_IOSTATUS). */
#define IOS_STS_M_OUT 0x03	/* Numero output. */
#define IOS_STS_M_IN 0x0C	/* Numero input. */
#define IOS_STS_M_NBTX 0x0F	/* Numero byte tx (moduli speciali). */
#define IOS_STS_M_WD 0x10	/* Modulo in fault. */
#define IOS_STS_M_ERR 0x20	/* Errore di comunicazione. */
#define IOS_STS_M_DIS 0x40	/* Modulo assente. */
#define IOS_STS_M_SPEC 0x80	/* Modulo speciale. */


#endif /* _IOSCNI_H */

