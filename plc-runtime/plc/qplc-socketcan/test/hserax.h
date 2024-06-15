/************************************************************
C.N.i.  
File di prototipi SERVER ASSI
************************************************************/


/*

	DOCUMENTAZIONE MINIMA SUL SERVER ASSI

Per utilizzare i servizi del SERVER ASSI occorre chiamare la AttachServerDM();
e passare alla init_col il pointer alla shared memory con il server dei dati 
macchina.  Questo pointer deve essere memorizzato nella struttura Sh_Address nel
campo shdatima.

ATTENZIONE:
Una lettura della CODAERRORI implica anche, dopo la lettura, un azzeramento
dell'area (2 bytes).
Per i parametri PASSO_GMM, ORIGINI, STAT_CEN, VEL_INT, ANT_INT, ACC_INT, RAGGIO,
 RAGGIO1, RAGGIO2, I_CENT, J_CENT,K_CENT il raggruppamento prende il significato
di centro;
Per il parametro G_ATT il raggruppamento deve contenere il centro mentre l'asse
deve contenere il valore della G.
Per i parametri VEL_INV, CEN_INV, il raggruppamento contiene il numero inverter.
Per il parametro OVERRIDE il raggruppamento deve contenere il numero override.
Per il parametro SAX_WATCH_DOG non servono inputs dal momento che scarichera' in
una tabella di puntatori (di dimensione MAX_LINEE) l'address dei watch_dog di
tutte le schede nel sistema. N.B. l'address varra' NULL in corrispondenza delle
schede inesistenti o di tipo AX.


N.B. Se si vuole leggere un dato indicizzato per asse dal gestore assi occorre
specificare solo il numero asse. Mentre se si vuole leggere un dato per linea..
.. per esempio la VERSIONE o il VSCOPIO occorre specificare solo la linea. 

NOTE send_command:
Se si invia un comando per linea (asse =-1) occorre specificare la linea mentre
se si specifica un numero asse, la linea viene calcolata automaticamente e il 
numero asse nel comando (pointer CMD +2) verra' aggiornato con il numero asse
all'interno della linea..



--------------------------------------------------------------------------------
int init_col(Sh_Address *);

Inizializza le shared memory (una per linea + una per lo smistatore) leggendo gl
i address scheda da eeser. Deposita gli address delle shmemory allocate nella struttura Sh_Address cui puntatore deve essere passato dal chiamante.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
unsigned char *ax_map_shared(int linea, int tipo)

Inizializza le shared memory dal lato gestore ASSI.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
int     ax_init_socket(int tipo,struct sockaddr_in *soap)

Inizializza il socket dal lato gestore ASSI.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
int read_parm(Sh_Address *,int codice_parametro, int asse, int linea, int raggruppamento, int *dato);
Serve per leggere un dato da una linea o dalla memoria dello smistatore.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
int write_parm(Sh_Address *,int codice_parametro, int asse, int linea, int raggruppamento, int *dato);
Serve per scrivere un parametro contenuto nella struttura in input nell'area dello smistatore.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
void    release_shared(Sh_Address *);
Serve per ripistinare le condizioni precedenti al lancio del server assi...
.. rilascia le shared memory e cancella i semafori allocati per gli assi.
Utile per la gestione del kill.
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
int send_command(Sh_Address *,int *buftra, int asse, int linea)
Invio comando in buftra con eventuale correzione numero asse nella linea.
Se asse = -1 nessuna correzione asse e viene utilizzato il numero della linea.
--------------------------------------------------------------------------------
*/

#ifndef __hserax_h
#define __hserax_h

#include <windows.h>

/* GG Thu May  2 11:11:04 MET DST 1996 : aggiunge definizioni
 mancanti in <sys/types.h> in modalita` POSIX (pezza ripugnante). */
#ifdef _POSIX_SOURCE
typedef unsigned short  u_short;
typedef unsigned short  ushort;
typedef unsigned char   u_char;
typedef unsigned long   u_long;
#endif


#define SAX_VER_SERVER  "2.0.1.9"       /* Versione server assi */
#define SAX_VERS        2019            /* Versione server per backup */
#define MAX_ASSI        60              /* Numero massimo assi gestibili */
#define MAX_ASSIxLINEA  16              /* Numero massimo assi x linea */
#define MAX_LINEE       16              /* Numero massimo linee */
#define MAX_INV         16              /* Numero massimo inverters */
#define MAX_CENTRI      16              /* Numero massimo centri */
#define MAX_OVERRIDE    4               /* Numero massimo override */
#define MAX_G           200             /* Numero massimo istruzioni G */
#define AX_INTERP       2               /* Nr. max insieme assi interpolanti */



/* Codici per l'accesso alla shared ram con il GESTORE ASSI */

#define SAX_STATO_ASSE                  0x00000001
#define SAX_QUOTA_REALE                 0x00000002
#define SAX_QUOTA_TEORICA               0x00000003
#define SAX_VELOCITA_REALE              0x00000004
#define SAX_VELOCITA_TEORICA            0x00000005
#define SAX_ERRORE_INSEGUIMENTO         0x00000006
#define SAX_QUOTA_TASTATORE             0x00000007
#define SAX_QUOTA_ASSE_ESTERNO          0x00000008
#define SAX_WATCH_DOG                   0x0000000A

#define SAX_VERSIONE                    0X0000000B
#define SAX_VSCOPIO                     0X0000000C
#define SAX_CODAERRORI                  0X0000000D
#define SAX_PASSOESEGUITO               0x0000000E
#define SAX_QUOTA_OBIETTIVO             0x0000000F
#define SAX_DATO_GENERICO               0x00000010
#define SAX_PUNTATORE_TX                0x00000011
#define SAX_CONTAMETRI                  0x00000012
#define SAX_INIT_ANALOGICA              0x00000013
#define SAX_RELEASE_ANALOGICA           0x00000014
#define SAX_INFO_VELOCI                 0x00000015
#define SAX_PUNTATORE_OSCILLO           0x00000016



/* Codici riferiti alla shared memory "SMISTATORE" */
#define SAX_CORR_TOT                    0x00000081
#define SAX_CORR_VIS                    0x00000082
#define SAX_CORR_ORIG                   0x00000083
#define SAX_CORR_PUNTE                  0x00000084
#define SAX_PASSO_GMM                   0x00000085
#define SAX_UTEN_RIF                    0x00000086
#define SAX_ORIGINI                     0x00000087
#define SAX_STAT_CEN                    0x00000088
#define SAX_VEL_INV                     0x00000089
#define SAX_CEN_INV                     0x0000008A
#define SAX_OVERRIDE                    0x0000008B
#define SAX_PRG_VEL                     0x0000008C
#define SAX_PRG_ANT                     0x0000008D
#define SAX_PRG_ACC                     0x0000008E
#define SAX_VEL_INT                     0x0000008F
#define SAX_ANT_INT                     0x00000090
#define SAX_ACC_INT                     0x00000091
#define SAX_RAGGIO                      0x00000092
#define SAX_RAGGIO1                     0x00000093
#define SAX_RAGGIO2                     0x00000094
#define SAX_I_CENT                      0x00000095
#define SAX_J_CENT                      0x00000096
#define SAX_K_CENT                      0x00000097
#define SAX_G_ATT                       0x00000098
#define SAX_AX_INTERP                   0x00000099
#define SAX_RESOLVER                    0x0000009A
#define SAX_FOTOQUOT                    0x0000009B
#define SAX_ORX                         0x0000009C
#define SAX_ORY                         0x0000009D
#define SAX_ORZ                         0x0000009E


/* equates di definizione CODICI ERRORI di ritorno */

#define SAX_TUTTO_OK            0x00000000
#define SAX_MANCA_INIZIALIZZAZIONE  0x00000001
#define SAX_CODICE_PARAMETRO_ERRATO 0x00000002
#define SAX_CE_POSTO            0X00000003      /* coda di tx in sh. mem*/
#define SAX_NON_CE_POSTO        0x00000004      /* coda di tx in sh. mem*/
#define SAX_SHARED_ERROR        0x00000005      /* prob sull'alloc sh. mem  */
#define SAX_SEM_ERROR           0x00000006      /* prob sull'alloc sh. mem  */
#define SAX_EESER_ERROR         0x00000007      /* prob sulla lettura eeser */
#define SAX_PARAMETRI_ERRATI    0x00000008      /* prob sui paramertri  */
#define SAX_INIT_LINEA_ERROR    0x00000009      /* prob sulla CPAX      */
#define SAX_TROPPI_BYTES_RX     0x0000000A      /* comando troppo lungo */
#define SAX_TROPPE_ANALOGICHE   0x0000000B      /* troppi I/O analogici */
#define SAX_ANALOGICA_NOT_FOUND 0x0000000C      /* pointer analog. non trovato*/



/*                      STRUTTURE                        */

typedef struct
	{
	int     linea;          /* un int di nr.linea per ogni asse */
	int     numlin;         /* un int di nr.asse sulla linea */
	int     address;
	}In_Ax;

typedef struct
	{
	int     *shsmist;               /* pointer alla shared con lo smist.*/
	int     descrsmis;              /* descrittore shared smist. x detach */
	char    *shline[MAX_LINEE+1];   /* pointers alle shared delle linee */

			 /* pointers alle shared oscilloscopi delle linee AY */
	char    *shline_osci[MAX_LINEE+1];

	int     descr[MAX_LINEE+1];     /* descrittori shared linee x detach */
	int     descr_osci[MAX_LINEE+1]; /* descr.shared oscil.linee x detach */

	int     semaf[MAX_LINEE+1];     /* descrittori semafori linee       */
	int     defli[MAX_LINEE+1];     /* un int di tipo per ogni linea    */
	In_Ax   infax[MAX_ASSI];        /* struttura dati per asse          */
	int     numero_linee;           /* un int di numero linee sistema   */
	int     numero_assi;            /* un int di numero assi nel CN     */
	int     numero_centri;          /* un int di numero centri del CN   */
	char    *shdatima;              /* pointer alla shared del server DM*/
	struct  sockaddr_in soap[MAX_LINEE+1]; /* struttura dati sockets    */
	int     sveglia[MAX_LINEE+1];   /* descrittore socket con AX        */
	char    stato_server[20];       /* stat=OK se l'init_col non ha dato*/
					/* errore in uscita                 */
	}Sh_Address;

typedef struct
	{
	char firma[4];
	short depo;
	short prel;
	short len;
	} shrd_mem;



int             init_col(Sh_Address *);
int             ax_mirror(Sh_Address *sh,int src_asse,int src_linea,int dst_asse
		,int dst_linea,int numero_asse);
unsigned char   *ax_map_shared(int linea, int tipo);
int             ax_init_socket(int tipo,struct sockaddr_in *soap);
int             read_parm(Sh_Address *,int, int, int, int, int *);
int             write_parm(Sh_Address *,int, int, int, int, int *);
int             send_command(Sh_Address *,int *, int, int);
void            release_shared(Sh_Address *);
void            ax_init_rx(shrd_mem *pun_shrd,int length);
int             ax_rx_shrd(shrd_mem *pun_shrd,int *bufric,int max_size,
								int semafo);
int             ax_rx_shrd_ns(shrd_mem *pun_shrd,int *bufric,int max_size);


#endif /* __hserax_h */ 
