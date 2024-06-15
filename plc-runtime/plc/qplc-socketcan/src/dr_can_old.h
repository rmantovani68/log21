/*
* dr_can.h 1.1 Sat Sep 13 02:06:35 CEST 2008
*
* 08/07/08 1.0 Aggiunta una funzione per agire su alcune opzioni
*              di funzionamento. Al momento e` possibile inibire
*              la rimappatura dei PDO sui moduli CNI Informatica.
*              Evitata l'inizializzazione degli assi sulla scheda K50.
* 13/09/08 1.1 Pesantissime modifiche per avere gli SDO asincroni.
*/

#ifndef DR_CAN_H_

typedef struct canboard_t canboard_t;
/* Descrittore di un comando SDO. Rappresenta una richiesta SDO
 dall'applicazione, candidata a produrre una transazione SDO. */
typedef struct sdo_t sdo_t;

canboard_t *canboard_open(int nch, int speed, int debug);
void canboard_close(canboard_t *cbr);
void canboard_send_sync(canboard_t *cbr);
void canboard_flush_output(canboard_t *cbr);
void canboard_flush_output_ex(canboard_t *cbr, int send_sync);
void canboard_set_output_bit(canboard_t *cbr, int mod, int bit, int val);
void canboard_set_output_byte(canboard_t *cbr, int mod, int b, int val);
void canboard_set_output_word(canboard_t *cbr, int mod, int b, int val);
unsigned char *canboard_get_input_image(canboard_t *cbr, int mod);
unsigned char *canboard_get_output_image(canboard_t *cbr, int mod);
unsigned short *canboard_get_ain_image(canboard_t *cbr, int mod);
unsigned short *canboard_get_aout_image(canboard_t *cbr, int mod);
long *canboard_get_enc_image(canboard_t *cbr, int mod);
int canboard_get_input_bit(canboard_t *cbr, int mod, int bit);
int canboard_read_dictionary(canboard_t *cbr, int mod,
                             int idx, int subidx,
                             void *data, int size);
int canboard_write_dictionary(canboard_t *cbr, int mod,
                             int idx, int subidx,
                             void *data, int size);
sdo_t *canboard_async_sdo(canboard_t *cbr, int mod,
                     int idx, int subidx,
                     void *data, unsigned int len,
                     int wr,
                     void (*cb)(sdo_t *sdo,
                                int err,
            	                unsigned int len,
                                unsigned int abort,
                                void *cb_arg),
                     void *cb_arg);
int canboard_async_sdo_cancel(sdo_t *sdo);
void canboard_schedule(canboard_t *cbr, unsigned long period);
int canboard_get_n_input(canboard_t *cbr, int mod);
int canboard_get_n_output(canboard_t *cbr, int mod);
int canboard_get_n_ain(canboard_t *cbr, int mod);
int canboard_get_n_aout(canboard_t *cbr, int mod);
int canboard_get_n_enc(canboard_t *cbr, int mod);
int canboard_activate_module(canboard_t *cbr, int mod);
int canboard_preop_module(canboard_t *cbr, int mod);
int canboard_reset_module(canboard_t *cbr, int mod);
int canboard_read_input(canboard_t *cbr, int mod);
int canboard_set_option(canboard_t *cbr, int option, int value);
#define CANBOARD_OPTION_NOREMAP_CNIINF 1

#define DR_CAN_H_
#endif

