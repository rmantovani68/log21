/*

    -------------------------------------------------------

    (Cnx) DEFINIZIONE COSTANTI  E PROTOTIPI FUNZIONI
        LIBRERIA PER GESTIONE COLLOQUIO VIA FTP
            06/11/1998

    -------------------------------------------------------

Versione  1.1:
	Introdotte le funzioni:
		remote_mkdir(), remote_rmdir(), rem_mkdir(), rem_rmdir()
Versione  1.2:
	Modificati alcuni codici errore.
Versione  1.3:
	Eliminata la ftp_setup
	Inserito per le funzioni "atomiche" i parametri
	della connessione ftp (host,login,password)
	Nelle funzioni "granulari" bisogna ora specificare il numero di
	socket restituito dalla hook_up , quest'ultima ha bisogno
	di avere i parametri della connessione ftp (host,login,password)
	Tutto cio' per avere la possibilita' di avere a disposizione n
	canali "granulari" contemporanei piu una di tipo atomico.
Versione  1.4:
	Aggiunte varie fclose(/dev/null) che mancavano in caso di errore.
	Succedeva che in caso l'errore persisteva finivano
	i file descriptor utilizzatbili (Segmentation fault)
	Eliminati i codici di ritorno superiori a short.
	Inserite le modifiche necessarie per windows (EXCEED).
	Aggiunta la possibilita' di utilizzare address IP invece che solo nomi.
	Eliminate alcune variabili globali inutili.
	Corretto un difetto nelle rem_mkdir() e rem_rmdir().
Versione  1.5:
	Eliminato un difetto nella rem_get() che si manifestava
	quando si richiedeva un file non esistente:
	in quel caso creava il file vuoto
	Aggiunte la funzione ret_inf_pcftp(int tipoinf).
	Aggiunte delle macro per gli errori.
Versione  1.6:
    Modificata le funzione rem_dir e remote_dir in modo che
    se il direttorio richiesto e' linkato simbolicamente con un altro
    restituisce comunque la lista dei file in esso contenuti.
    Modificate le lunghezze dei nomi dei file da 50 caratteri a LUNG_PATH
Versione  2.0:
	Modifiche alla recv in modo che questa sia di tipo non bloccante
	e intercettato l'errno 11 in modo da uscire solo in caso di connessione
	fallita o in generale in caso di errore.
	1.	Modificata la funzione hang_off, con l'aggiunta della
		spedizione dell'istruzione QUIT.		
 	2.	Nella linea che contiene LIST, eliminato un blank e la ripetizione
		della stringa col nome del	 direttorio remoto, che peraltro non
		serve piu' in quanto ho fatto precedere il tutto da  un CWD.
 	3.	Samuele ha generato un filtro che elimina i carattere \r che possono
		arrivare da DOS/WINDOWS	 (chiedere meglio a lui)
 	4.	Ora la hook_up setta il socket di comunicazione come NONBLOCK con
		una fcntl, per consentire il  punto 5.
 	5.	Rimaneggiata la recv che ora consente di non dover attendere tutto
		il timeout per considerare morta la connessione, bensi' attende solo
		per DEFUNCT ( => settabile a piacere )		
 	6.	Samuele che ha eliminato i caratteri-barra (\) in tutte le funzioni
		che ha ritenuto opportuno (chedere meglio a lui)
 	7.	Nella writefile ora c'e' una writen invece di una send
 	8.	Ora il socket relativo ai dati nella funzione rem_put e'
		settato come bloccante			
 	9.	Quando trovo un errore di ritorno della categoria 5.. chiudo
		con l'hang_off invece che close	
 	10.	Rimaneggiate le funz. remote_remove e rem_remove,
		ma in modo "trasparente"			
 	11.	In commread ho eliminato il meccanismo del tentativo reiterato
 	12.	Ora il timeout e' 2 minuti. Se lo si abbassa, si rischia che
		una connessione lenta (slip) venga chiusa anche se non ci sono i motivi
Versione  2.1:
	Corretto un difetto sulle chusure in caso di errore del server
Versione  2.2:
	1.	Aggiunta la funzione rem_dir2, che ha parametri di chiamata identici
		alla rem_dir. Ha pero' la possibilita' di avere un nome file alla
		fine del path.
		Si ricorda pero' che la rem_dir2 - al contrario della rem_dir -
		NON puo' "vedere" direttori linkati.
	2.	Aggiunta la funzione granulare rem_cd, che si chiama nel seguente modo :
		   int rem_cd(char remdir[], int socket);
	3.	Aggiunta la funzione atomica remote_cd, che si chiama nel seguente modo:
		   int remote_cd(char *remotedir, char *host, char *login, char *passw);
	Essendo tutte e tre funzionalita' nuove rispetto alla 2.1, vanno TESTATE !
Versione  2.3:
	1.	Correzione alle 3 funzioni "granulari" : REM_MKDIR, REM_RMDIR, REM_CD .
		Esse facevano uso di un fantomatico canale dati che non veniva mai 
		usato e mai richiuso. In pratica veniva allocato un socket ogni volta
		che veniva chiamata una di queste funzioni, che veniva rilasciato solo
		all'uscita dell'applicativo. Ora non piu'. 
*/

#define VERSPCFTP	"2.3"					/*versione libreria*/

int hook_up(char *host, char *login, char *passw);
int rem_put(char *file, char *locdir, int zocket);
int rem_get(char file[], char locdir[], int zocket);
int rem_remove(char f[], char rd[], int s);
int rem_cd(char remdir[], int s);
int rem_mkdir(char *remdir, int s);
int rem_rmdir(char *remdir, int s);
int rem_dir(char *rd, FILE *out, int s);
int rem_dir2(char *rd, FILE *out, int s);
int hang_off(int s);
int receive_file(char *f,char *rd,char *ld,char *host,char *login,char *passw);
int transmit_file(char *f,char *rd,char *ld,char *host,char *login,char *passw);
int remote_remove(char *f, char *rd, char *host, char *login, char *passw);
int remote_dir(char *rd, FILE *out, char *host, char *login, char *passw);
int remote_mkdir(char *remdir, char *host, char *login, char *passw);
int remote_cd(char *rd,char *host, char *login, char *passw);
int remote_rmdir(char *remdir, char *host, char *login, char *passw);
int directory_read(int s, char buffer[], FILE *out);
int ret_inf_pcftp(int tipo);


#define LUNG_PATH                   256
#define	RET_CODE_PCFTP(err)			(-(err))
#define	IF_NOFILE_PCFTP(err)		(((-err)%1000)==550)
#define	IF_NOLOGIN_PCFTP(err)		(((-err)%1000)==530)
#define	INF_PCFTP_ERRNO				0
#define	INF_PCFTP_CLASS				1


/*
					--------------------------
					DESCRIZIONE delle FUNZIONI
					--------------------------

Sono state sviluppate due classi di funzioni, per soddisfare esigenze diverse:
quelle che d'ora in poi chiameremo "atomiche", e quelle che d'ora in poi
chiameremo "granulari".
Siccome l'handshake tipico dell'FTP prevede una fase di connessione,
una fase di flusso di dati/controlli e infine una fase di sconnessione,
chiameremo atomiche quelle funzioni che con una sola chiamata effettuano
tutte e tre questi passaggi.
Le funzioni granulari, al contrario, sono funzioni che necessitano
di una connessione gia' stabilita precedentemente,
e che non provvedono alla sconnessione.
Cio' implica quindi la presenza di istruzioni di inizio e fine connessione.
Queste ultime funzioni sono particolarmente utili in caso di trasferimenti
multipli, in modo similare all' mget e all' mput tipici dell'FTP interattivo. 
Prerogativa caratteristica poi delle istruzioni granulari, consiste nella
possibilita' di effettuare n connessioni contemporanee, con n hosts remoti.

Le funzioni di tipo atomico attualmente disponibili sono :

int receive_file(	char *nomefile, char *direttorioremoto,
					char * direttoriolocale, char * nomehost,
					char * login, char * password);
int transmit_file(	char *nomefile, char *direttorioremoto,
					char * direttoriolocale, char * nomehost,
					char * login, char * password);
int remote_remove(	char *nomefile, char *direttorioremoto,
					char * nomehost, char * login, char * password);
int remote_dir(		char *direttorioremoto, FILE *stream,
					char * nomehost, char * login, char * password);
int remote_mkdir(	char *direttorioremoto, char * nomehost,
					char * login, char * password);
int remote_rmdir(	char *direttorioremoto, char * nomehost,
					char * login, char * password);


Con le prime due funzioni, e' possibile copiare files da e per l'host remoto. 
Con remote_remove e' possibile cancellare un file esistente sull'host remoto.
Con remote_dir e' possibile ricevere il listato della directory specificata
relativamente all'host remoto.
Con le ultime due funzioni e' possibile creare ed eliminare
directory presso l'host remoto.
Si debbono specificare tutti i parametri, per avere la liberta'
di non usare istruzioni aggiuntive di navigazione nel filesystem.

ESEMPIO: 
	receive_file( "pippo.txt", "/home/francis", "~", "pluto", "guest", "wq91A");

copia nella propria home il file pippo.txt che si trova nella directory
/home/francis dell'host pluto, allogandosi con login guest e password wq91A.


L'istruzione di remote_dir, ha la prerogativa di avere la necessita' del
parametro "stream".
Con esso e' possibile specificare dove ridirigere il listato che sta
arrivando via rete dall'host remoto. 
ESEMPIO:
		remote_dir("/home/guest", stdout, "pluto", "guest", "wq91A");

genera il listato della directory /home/guest dell'host direttamente a video.

Le istruzioni di tipo granulare attualmente disponibili sono :

int  hook_up	( char *host, char * login, char * passwd );
int  hang_off	( int socket );

int  rem_put	( char * file, char *remotedir, char *localdir, int socket );
int  rem_get	( char * file, char *remotedir, char *localdir, int socket  );
int  rem_dir	( char *remotedir, FILE *stream, int socket  );
int  rem_remove	( char * file, char *remotedir, int socket  );
int  rem_rmdir	( char *remotedir, int socket );
int  rem_mkdir	( char *remotedir,int socket );

Si puo' notare la similitudine con le istruzioni di tipo atomico,
anche se le prime due non trovano corrispondenza.
Le prime due istruzioni devono essere sempre l'inizio e la fine della
sequenza composta dalle altre.  In buona sostanza si puo' affermare
che l'istruzione atomica:

	receive_file( "pippo.txt", "/home/francis", "~", "pluto", "guest", "wq91A");

corrisponde alla sequenza
	. . . . . . 
	mysock=hook_up("pluto", "guest", "wq91A");
	rem_put( "pippo.txt", "/home/guest", "~", mysock);
	hang_off(mysock);
	. . . . . . 

NOTE:
	E' possibile dalla versione 1.4 specificare come
	nomehost anche un indirizzo IP.


					---------------------------------------
					CODICI ERRORE di RITORNO delle FUNZIONI
					---------------------------------------


Ciascuna funzione ritorna un codice che se e' negativo e' indice di errore.
Tale codice di errore puo essere normalizzato tramite la RET_CODE_PCFTP(err)
dove err e' il valore di ritorno della funzione.
Nel caso di richiesta di un file che non c'e' o che non ha i permessi di
lettura e' possibile discriminare questo errore dagli altri tramite
la chiamata IF_NOFILE_PCFTP(err) dove err e' il valore
di ritorno della funzione.
In caso di errore e' possibile esaminare il contenuto della copia di errno
tramite la chiamata a funzione : ret_inf_pcftp(INF_PCFTP_ERRNO).
E' anche possibile risalire all' azione che ha generato l'errore tramite
la chiamata a funzione : ret_inf_pcftp(INF_PCFTP_CLASS) :
		0=hook_up 1=rem_put 2=rem_get 3=rem_remove 4=rem_dir
		5=rem_mkdir 6=rem_rmdir 7=hang_off

_______________________________________________________________

receive_file	( hook_up )
				( rem_get )

transmit_file	( hook_up )
				( rem_put )

remote_dir		( hook_up )
				( rem_dir )

remote_remove	( hook_up )
				( - writen )
				( - comread )

remote_rmdir	( hook_up )
				( rem_rmdir )

remote_mkdir	( hook_up )
				( rem_mkdir )

______________________________________________________________

rem_remove		( -writen - 30 )
				( -commread )

hook_up			( - 2100 ) gethostbyname
				( - 2200 ) socket
				( - 2300 ) connect
				( - 2400 ) gethostbyaddr
				( - 2789 ) setsockopt REUSEADDR
				( - 5500 ) getsockbyname
				( - 5600 ) getpeername
				( - 5700 ) fcntl
				( - commread      ) lettura rich. login
				( - writen - 12 )   spediz. login
				( - commread      ) lettura rich. password
				( - writen - 14 )   spediz. password
				( - commread      ) lettura dopo password
				( - writen - 15 )   spediz. comando BIN
				( - commread      ) lettura dopo BIN

rem_get			( - 2200 ) socket
				( - 1700 ) getsockname
				( - writen - 77 ) spediz. comando PORT
				( - commread - 6 ) lettura dopo PORT
				( - 1800 ) fallimento apertura file scrittura
				( - writen - 21 ) spediz. comando RETR
				( - 7600 ) fcntl
				( - 7800 ) la recv MSG_PEEK e' fallita
				( - 3000 - ftperror ) errore FTP
				( - 2000 ) accept
				( - commread  ) attesa segnale 150
				( - dataread - 9 ) attesa dati
				( - commread  ) attesa segnale 226

rem_put			( - 2200 ) socket
				( - 1700 ) getsockname
				( - writen - 77 ) spediz. comando PORT
				( - commread - 6 ) lettura dopo PORT
				( - 1800 ) fallimento apertura file lettura
				( - writen - 21 ) spediz. comando STOR
				( - 7600 ) fcntl
				( - 7800 ) la recv MSG_PEEK e' fallita
				( - 3000 - ftperror ) errore FTP
				( - 2000 ) accept
				( - commread   ) attesa segnale 150
				( - dataread - 9 ) attesa dati
				( - commread   ) attesa segnale 226

rem_dir			( - 2200 ) socket
				( - 1700 ) getsockname
				( - writen - 77 ) spediz. comando PORT
				( - commread - 6 ) lettura dopo PORT
				( - writen - 21 ) spediz. comando DIR
				( - 7600 ) fcntl
				( - 7800 ) la recv MSG_PEEK e' fallita
				( - 3000 - ftperror ) errore FTP
				( - 2000 ) accept
				( - commread     ) attesa segnale 150
				( - dataread - 9 ) attesa dati
				( - commread     ) attesa segnale 226

rem_mkdir		( - 2200 ) socket
				( - 1700 ) getsockname
				( - writen - 21 ) spediz. comando MKDIR
				( - 7800 ) la recv MSG_PEEK e' fallita
				( - 3000 - ftperror ) errore FTP
				( - commread     ) attesa segnale 226

rem_rmdir		( - 2200 ) socket
				( - 1700 ) getsockname
				( - writen - 21 ) spediz. comando RMDIR
				( - 7800 ) la recv MSG_PEEK e' fallita
				( - 3000 - ftperror ) errore FTP
				( - commread     ) attesa segnale 226

hang_off		( - 1 )  non riesco a chiudere il socket

_______________________________________________________________

directory_read	( 7100 ) recv fallita
				( 7200 ) fwrite(stdout) fallita

commread		( 7400 ) recv fallita
				( 7000 + ftperror ) errore FTP
		
dataread		( 7500 ) recv fallita
				( 7300 )                   
				( 7900 )                   

writen			( 7700 ) send fallita

writefile		( 7800 ) send fallita

_______________________________________________________________

ftp				( 421 ) servizio temporaneamente non disponibile,
						il server remoto ha chiuso la connessione
				( 425 ) connessione non apribile
				( 426 ) trasferimento abortito, chiusa la comununicazione dati
				( 451 ) connessione non apribile
				( 452 ) scrittura file fallita

				( 502 ) errore nel file di input
				( 503 ) login with user first
				( 530 ) login errata, user sconosciuto, ...
				( 550 ) lettura file fallita, permessi negati, ...
				( 551 ) error on input file
				( 553 ) TYPE E not implemented
_______________________________________________________________

*/
