
/*
* Questo file contiene l'algoritmo di salvataggio della configurazione
* degli I/O sulla RAM non volatile.
*/

#include <ios.h>
#include <nvram.h>

/* Codice di validazione della struttura registrata in ram non volatile. */

#define PLC_NVRAM_CHECK_V1 0xbe191061

/* Struttura immagazzinata nella zona privata della RAM non volatile.
 In previsione di futuri sconvolgimenti, il primo elemento della struttura
 e` un codice che fa sia da firma che da versione, e permette di decidere
 come interpretare la struttura stessa. */

union nvram_ioconf_t {

/* VERSIONE 1 */
struct {
/* Se questo non vale PLC_NVRAM_CHECK_V1, la struttura non e` da considerare
 valida. */
	unsigned long check;
/* Tabella dei flag di presenza dei controllori : 8 bit per 8 schede (ed e`
 anche troppo !). Se una cpu e` presente, dovrebbe esistere (compatibilmente
 con lo spazio disponibile nella ram non volatile) l'immagine dei byte
 di stato del controllore. */
	unsigned char board[8];
/* Tabella delle immagini dei controllori. Una per ogni bit acceso
 in "board" */
	unsigned char images[1][32];
/* NON AGGIUNGERE CAMPI DI SEGUITO ! */
} v1;

/* VERSIONE 2, se arriva */
struct {
/* Se questo non vale PLC_NVRAM_CHECK_V2, la struttura non e` da considerare
 valida. */
	unsigned long check;
/* ??? */
} v2;

/* ecc... */

};

/* Questi sono stati generati dalle "iosOpen". */
extern unsigned char * NvRam;
extern unsigned char ** CNI_ios[];

/*
* Funzione plcSaveIO()
* --------------------
*
*  Questa funzione salva la configurazione in NVRAM.
*  Vale 1 se tutto e` andato bene, 0 se non e` installata la RAM non
* volatile, -1 se non c'e` spazio sulla RAM non volatile.
*/

int plcSaveIO(void)
{
union nvram_ioconf_t * io;
int i,j,k,m,rv,n;

	if (! NvRam) {
		return 0;
	}

	io = (union nvram_ioconf_t *) (NvRam + NVRAM_OFF_PLC);

	io -> v1.check = PLC_NVRAM_CHECK_V1;

	n = 0;

	for (i = 0; i < sizeof(io -> v1.board); ++i)
		io -> v1.board[i] = 0;
	for (i = 0; i < sizeof(io -> v1.board); ++i) {
		if (CNI_ios[i]) {
			for (j = 0, m = 1; j < 8; ++j, m <<= 1) {
				if (CNI_ios[i][j]) {
					if (n >= (NVRAM_SIZ_PLC
					          - sizeof(*io))/sizeof(io -> v1.images[0])+1)
						return -1;
					io -> v1.board[i] |= m;
					for (k = 0; k < sizeof(io -> v1.images[0]); ++k) {
						io->v1.images[n][k] =
						     CNI_ios[i][j][IOS_IOSTATUS + k]
						   & (IOS_STS_M_SPEC | IOS_STS_M_DIS | IOS_STS_M_NBTX);
					}
					++n;
				}
			}
		}
	}

	return 1;
}

