
/* Come si scrive... */

{
ios_lae_t *p;
int i,idx;
long cmd;

	for (p = dr -> lae_out_list; p; p = p -> next) {

		for (i = 0; i < p -> nused; ++i) {

			idx = p -> usedidx[i];

			cmd = (0x10 + idx) /* Codice del comando */
			if (p -> delay[idx].var)
				cmd |= (*(p -> delay[idx].var)) >> 8;
			if (p -> trpol[idx].var && (*(p -> trpol[idx].var))
				cmd |= 0x1000000;
			if (p -> oupol[idx].var && (*(p -> oupol[idx].var))
				cmd |= 0x2000000;

			if (cmd != p -> dl_pol_img[idx]) {
				if (*(p -> addr) == 0) {

			/* Con questo si tiene occupato il modulo. Eventuali
			 variazioni delle uscite ordinarie non avranno effetto
			 finche` non sono stati elaborati tutti i comandi
			 di configurazione pendenti. E` importante che
			 questo tratto di codice preceda quello in cui si
			 elaborano le uscite ordinarie SELCO/LAE. */

					p -> addr[1] = (cmd >> 8) & 0xFF;
					p -> addr[2] = (cmd >> 16) & 0xFF;
					p -> addr[3] = (cmd >> 24) & 0xFF;
					p -> addr[0] = cmd & 0xFF;

					p -> dl_pol_img[idx] = cmd;
				}
				else {

			/* Modulo occupato. Conviene continuare?
			  Forse no: Se e` occupato ora, quante probabilita`
			 ci sono che non lo sia tra una manciata di
			 microsecondi? */

				}

		/* USCITA DAL CICLO INTERNO */

				break;

			}

			cmd = (0x18 + idx) /* Codice del comando */
			if (p -> pulse[idx].var)
				cmd |= (*(p -> pulse[idx].var)) >> 8;

			if (cmd != p -> pulse_img[idx]) {
				if (*(p -> addr) == 0) {

			/* Con questo si tiene occupato il modulo. Eventuali
			 variazioni delle uscite ordinarie non avranno effetto
			 finche` non sono stati elaborati tutti i comandi
			 di configurazione pendenti. E` importante che
			 questo tratto di codice preceda quello in cui si
			 elaborano le uscite ordinarie SELCO/LAE. */

					p -> addr[1] = (cmd >> 8) & 0xFF;
					p -> addr[2] = (cmd >> 16) & 0xFF;
					p -> addr[3] = (cmd >> 24) & 0xFF;
					p -> addr[0] = cmd & 0xFF;

					p -> pulse_img[idx] = cmd;
				}
				else {

			/* Modulo occupato. Conviene continuare?
			  Forse no: Se e` occupato ora, quante probabilita`
			 ci sono che non lo sia tra una manciata di
			 microsecondi? */

				}

		/* USCITA DAL CICLO INTERNO */

				break;

			}
		}
	}
}

