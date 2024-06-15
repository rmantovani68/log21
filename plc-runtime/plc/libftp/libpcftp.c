/*
* libpcftp.c
* funzioni di gestione ftp parte client
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<signal.h>
#include	<sys/time.h>
#include	<fcntl.h>
#include	<errno.h>
#ifndef EXCEED
#include	<unistd.h>
#endif
#include	<string.h>
#ifdef		Lynx
#include	<pthread.h>
#endif
#ifndef EXCEED
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<arpa/inet.h>
#else
#include	<winsock.h>
#include	<io.h>
#endif
#include	"libpcftp.h"

#define SERV_TCP_PORT_DATA	20	 	/* server well known port for FTP data */
#define SERV_TCP_PORT		21	 	    /* server well known port for FTP command */
#define BUFFERSIZE		1024		    /* buffer dimension in byte */
#define DEFUNCT2		120000000	    /* tempo per dichiarare morta la conn. */
#define DEFUNCT			120000000	    /* tempo per dichiarare morta la conn. (si puo ridurre) */

#ifndef EXCEED
#define NO_OUT				"/dev/null"
#define	closesocket		close	 /*SB*/
#else
#define NO_OUT				"NUL"
#endif

/* #define DEBUG */

#ifdef DEBUG
	FILE *fpDebug;
	char *szDebugFileName="ftpDebug.trc";
#endif

int commread(int s);
int dataread(int f, char buff[], FILE *fd);
int writen(register int fd, register char *ptr, register int nbytes);
int FTPSendCmd(register int fd, register char *ptr, register int nbytes);
int writefile(int sock, char buff[], FILE *fd);
#ifdef EXCEED
int	bind_socket(SOCKET,LPSOCKADDR_IN);
#endif
double aspetta(void);
int nnrecv(int sock, char stringa[], int numb, int flag);

extern char		pf_buffer[];
extern char		pf_local_host_IPaddr[];
extern int		pcftp_errno;
extern int		pcftp_error_class;
extern int		helpflag;

/*LIB*/

/****************************************************************/
/************** VARIABILI GLOBALI *******************************/
/****************************************************************/

char	pf_buffer[BUFFERSIZE]="";
char	pf_local_host_IPaddr[50];
int		pcftp_error_class;
int		pcftp_errno;
int		helpflag;


/*LIB*/

/***********************************************************/
/******************* RECEIVE_FILE FUNCTION *****************/
/***********************************************************/

int receive_file(char *f, char *rd, char *ld, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	mysocket;
	int	err=0;

#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"RECEIVE FILE [%s][%s]\n",rd,f);
		fprintf(fpDebug,"----------------------------------------\n");
		fclose(fpDebug);
	}
#endif

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<0)
		return(mysocket);

	err=rem_cd(rd, mysocket);
	if(err<0) {
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999) {
		hang_off(mysocket);
		return(-err);
	}

	err=rem_get(f, ld, mysocket);
	if(err<0) {
#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"ERROR : %d [%s]",errno,strerror(errno));
		fprintf(fpDebug,"----------------------------------------\n");
		fclose(fpDebug);
	}
#endif
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999)
	{
		hang_off(mysocket);
		return(-err);
	}
	hang_off(mysocket);
#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"----------------------------------------\n");
		fclose(fpDebug);
	}
#endif
	return(0);
}

/*LIB*/

/************************************************************/
/******************* TRANSMIT_FILE FUNCTION *****************/
/************************************************************/

int transmit_file(char *f, char *rd, char *ld, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	err=0;
	int	mysocket;

#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"TRANSMIT FILE [%s][%s]\n",rd,f);
		fprintf(fpDebug,"----------------------------------------\n");
		fclose(fpDebug);
	}
#endif
	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_cd(rd, mysocket);
	if(err<0) {
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999) {
		hang_off(mysocket);
		return(-err);
	}

	err=rem_put(f, ld, mysocket);
	if(err<0) {
#ifdef DEBUG
		if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
			fprintf(fpDebug,"ERROR : %d [%s]",errno,strerror(errno));
			fprintf(fpDebug,"----------------------------------------\n");
			fclose(fpDebug);
		}
#endif
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999)
	{
		hang_off(mysocket);
		return(-err);
	}
	hang_off(mysocket);
#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"----------------------------------------\n");
		fclose(fpDebug);
	}
#endif
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REMOTE_DIR FUNCTION ********************/
/************************************************************/

int remote_dir(char *rd, FILE *out, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	err=0;
	int	mysocket;

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_dir(rd, out, mysocket);
	if(err<0)
	{
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999)
	{
		hang_off(mysocket);
		return(-err);
	}

	hang_off(mysocket);
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REMOTE_CD    FUNCTION ******************/
/************************************************************/

int remote_cd(char *rd, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int     err=0;
	int     mysocket;

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_cd(rd, mysocket);
	if(err<0)
	{
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999)
	{
		hang_off(mysocket);
		return(-err);
	}

	hang_off(mysocket);
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REMOTE_REMOVE FUNCTION *****************/
/************************************************************/

int remote_remove(char f[], char rd[], char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	err=0;
	int	mysocket;

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_remove(f, rd, mysocket);
	if(err<0)
	{
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999)
	{
		hang_off(mysocket);
		return(-err);
	}

	hang_off(mysocket);
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REM_CD     FUNCTION ********************/
/************************************************************/
int rem_cd(char remdir[], int zocket)
{
	int                   lungh, get_len, gsn1,port_ret, ret_code;
	int                   ftperr;
	char                  nomefile_remoto[LUNG_PATH], test[3], stringa[1024];
	u_short               loc_data_port;
	char                  retrieve[LUNG_PATH+50];
	struct sockaddr_in    new_client_address;

	errno=0;
	pcftp_error_class=5;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/
	lungh=sizeof(new_client_address);

	strcpy(retrieve, "CWD ");

	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (remdir[strlen(remdir)-1]=='/') remdir[strlen(remdir)-1]=0;

	strcpy(nomefile_remoto, remdir);

	strcat(retrieve, nomefile_remoto);
	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);
	port_ret=FTPSendCmd(zocket, retrieve, get_len);     /*invio comando CWD*/
	if(port_ret>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(zocket, test, 3, MSG_PEEK);     /*testo eventuali errori*/
	if(port_ret<0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( /*test[0]=='4' ||*/ test[0]=='5' ) {
		pcftp_errno=errno;
		ftperr=atoi(test);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' ) {
		/*fclose(pf_fdnull);*/
		return(9999);
	}
	/*aspetto il segnale 257*/
	if( (ret_code=commread(zocket))>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	/*fclose(pf_fdnull);*/
	pcftp_errno=errno;
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REMOTE_MKDIR FUNCTION ******************/
/************************************************************/

int remote_mkdir(char *rd, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	err=0;
	int	mysocket;

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_mkdir(rd, mysocket);
	if(err<0) {
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999) {
		hang_off(mysocket);
		return(-err);
	}

	hang_off(mysocket);
	return(0);
}

/*LIB*/

/************************************************************/
/******************* REMOTE_RMDIR FUNCTION ******************/
/************************************************************/

int remote_rmdir(char *rd, char *pf_hostp, char *pf_login, char *pf_passwd)
{
	int	err=0;
	int	mysocket;

	mysocket=hook_up(pf_hostp, pf_login, pf_passwd);
	if(mysocket<=0)
		return(mysocket);

	err=rem_rmdir(rd, mysocket);
	if(err<0) {
		if( -err > 3000 && -err < 4000 )
			hang_off(mysocket);
		else
			closesocket(mysocket);
		return(err);
	}
	if(err==9999) {
		hang_off(mysocket);
		return(-err);
	}

	hang_off(mysocket);
	return(0);
}

/*LIB*/

/************************************************************************/
/*************** REM_MKDIR FUNCTION *************************************/
/************************************************************************/

int rem_mkdir(char remdir[], int zocket)
{
	int					lungh, get_len, gsn1,port_ret, ret_code;
	int					ftperr;
	char				nomefile_remoto[LUNG_PATH], test[3], stringa[1024];
	u_short				loc_data_port;
	char				retrieve[LUNG_PATH+50];
	struct sockaddr_in	new_client_address;
	/*FILE 				*pf_fdnull;*/

	errno=0;
	pcftp_error_class=5;
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/
	lungh=sizeof(new_client_address);

	strcpy(retrieve, "MKD ");

	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (remdir[strlen(remdir)-1]=='/') remdir[strlen(remdir)-1]=0;

	strcpy(nomefile_remoto, remdir);

	strcat(retrieve, nomefile_remoto);
	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);
	port_ret=FTPSendCmd(zocket, retrieve, get_len);     /*invio comando MKD*/
	if(port_ret>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(zocket, test, 3, MSG_PEEK);	/*testo eventuali errori*/
	if(port_ret<0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' ) {
		pcftp_errno=errno;
		ftperr=atoi(test);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' ) {
		/*fclose(pf_fdnull);*/
		return(9999);
	}

	/*aspetto il segnale 257*/
	if( (ret_code=commread(zocket))>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	/*fclose(pf_fdnull);*/
	pcftp_errno=errno;
	return(0);
}

/*LIB*/

/************************************************************************/
/***************** REM_RMDIR FUNCTION ***********************************/
/************************************************************************/

int rem_rmdir(char remdir[], int zocket)
{
	int					lungh, get_len, gsn1, port_ret, ret_code;
	int					ftperr;
	char				nomefile_remoto[LUNG_PATH], test[3], stringa[1024];
	u_short				loc_data_port;
	char				retrieve[LUNG_PATH+50];
	struct sockaddr_in	new_client_address;
	/*FILE 				*pf_fdnull;*/

	pcftp_error_class=6;
	errno=0;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/
	lungh=sizeof(new_client_address);

	strcpy(retrieve, "RMD ");
	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (remdir[strlen(remdir)-1]=='/') remdir[strlen(remdir)-1]=0;

	strcpy(nomefile_remoto, remdir);

	strcat(retrieve, nomefile_remoto);
	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);
	port_ret=FTPSendCmd(zocket, retrieve, get_len); /*invio comando RMD*/
	if(port_ret>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(zocket, test, 3, MSG_PEEK);	/*testo eventuali errori*/
	if(port_ret<0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' ) {
		pcftp_errno=errno;
		ftperr=atoi(test);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' ) {
		/*fclose(pf_fdnull);*/
		return(9999);
	}

	/*aspetto il segnale 257*/
	if( (ret_code=commread(zocket))>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	/*fclose(pf_fdnull);*/
	pcftp_errno=errno;
	return(0);
}

/*LIB*/

/************************************************************************/
/***************** REM_REMOVE FUNCTION ***********************************/
/************************************************************************/
int rem_remove(char f[], char rd[], int zocket)
{
	int		port_ret=0, ret_code=0, del_len=0, ftperr;
	char		test[3], stringa[1024], del_command[LUNG_PATH+50]="";
	/*FILE 		*pf_fdnull;*/

	pcftp_error_class=3;
	errno=0;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/

	strcpy(del_command, "DELE ");

	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (rd[strlen(rd)-1]=='/') rd[strlen(rd)-1]=0;

	strcat(del_command, rd);
	strcat(del_command, "/");
	strcat(del_command, f);
	strcat(del_command, "\r\n" );
	del_len=strlen(del_command);

	port_ret=FTPSendCmd(zocket, del_command, del_len);
	if(port_ret>0) {
		pcftp_errno=errno;
		closesocket(zocket);
		return(-port_ret-30);
	}
	port_ret=nnrecv(zocket, test, 3, MSG_PEEK);	/*testo eventuali errori*/
	if(port_ret<0) {
		pcftp_errno=errno;
		return(-7800);
	}
	if( test[0]=='5' ) {
		pcftp_errno=errno;
		ftperr=atoi(test);
		return(-3000-ftperr);
	}
	if( test[0]=='4' ) {
		return(9999);
	}
	if( (ret_code=commread(zocket))>0) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(zocket);
		return(ret_code);
	}

	pcftp_errno=errno;
	return(0);
}





/*LIB*/

/*************************************************************/
/***************** HOOK_UP FUNCTION **************************/
/*************************************************************/

int hook_up(char *pf_hostp, char *pf_login, char *pf_password)
{
	char				*myptr, m1[50], m2[50];
	char				lha1[10],lha2[10],lha3[10],lha4[10];
	char				serv_host_IPaddr[20]="", loc_comm_port_str[8]="";
	char				rem_comm_port_str[8]="", sha1[10],sha2[10],sha3[10],sha4[10];
	int					lungh, gsn, gpn, ret_code, log_len, log_ret, port_ret;
	struct sockaddr_in	serv_addr, local_address, remote_address;
	u_short				loc_comm_port, rem_comm_port;
	struct hostent		*server_host;
	/*FILE 				*pf_fdnull;*/
	int					commsockfd;
	int					pf_flag=1;
	u_long				maxaddr;

	errno=0;
	pcftp_error_class=0;

	if ((int)(maxaddr = inet_addr(pf_hostp)) == -1) {
		/* il parametro era un nome quindi cerco nella tab hosts*/
		if((server_host=gethostbyname(pf_hostp))==NULL) {
			pcftp_errno=errno;
			/*fclose(pf_fdnull);*/
			return(-2100);
		}
		myptr=*(server_host->h_addr_list);             /* prendo il punt a IP addr*/
		/* resetto i campi di serv_addr poi li setto con i dati di : */
		/*1 - famiglia di indirizzi 2 - porta del servizio remoto 3 - IP addr rem*/
		memset((char *) &serv_addr, '\0', sizeof(serv_addr));
		serv_addr.sin_family      	= AF_INET;
		serv_addr.sin_port        	= htons(SERV_TCP_PORT);
		/*
		* -------------------------------------------
		* serv_addr.sin_addr.s_net	= *(myptr+0);
		* serv_addr.sin_addr.s_host	= *(myptr+1);
		* serv_addr.sin_addr.s_lh		= *(myptr+2);
		* serv_addr.sin_addr.s_impno	= *(myptr+3);
		* -------------------------------------------
		*/
		/*
		* rm 28-09-1999 : non corrisponde alla struttura (vedi /usr/include/netinet/in.h)
		*/
		memcpy(&(serv_addr.sin_addr.s_addr),server_host->h_addr_list[0],sizeof(serv_addr.sin_addr.s_addr));
	} else {
		/* il parametro era un indirizzo vero */
		/* resetto i campi di serv_addr poi li setto con i dati di : */
		/*1 - famiglia di indirizzi 2 - porta del servizio remoto 3 - IP addr rem*/
		memset((char *) &serv_addr, '\0', sizeof(serv_addr));
		serv_addr.sin_family      	= AF_INET;
		serv_addr.sin_port        	= htons(SERV_TCP_PORT);
		serv_addr.sin_addr.s_addr	= maxaddr;
	}

	/* chiedo un socket */
	if ( (commsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-2200);
	}

	/*mi connetto*/
	if ( connect(commsockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(commsockfd);
		return(-2300);
	}

	if( setsockopt(commsockfd, SOL_SOCKET, SO_REUSEADDR,
		               (char *) &pf_flag, sizeof(pf_flag) ) <0 )
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-2789);
	}

	if( fcntl(commsockfd, F_SETFL, O_NONBLOCK) < 0 )
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-5700);
	}

	if( setsockopt(commsockfd, SOL_SOCKET, SO_KEEPALIVE,
		               (char *) &pf_flag, sizeof(pf_flag) ) <0 )
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-2789);
	}

	lungh=sizeof(local_address);
	gsn = getsockname(commsockfd,  (struct sockaddr *)&local_address,  &lungh);

	/*acquis IP e PORT locali*/
	if(gsn==-1)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-5500);
	}

	gpn = getpeername(commsockfd,  (struct sockaddr *)&remote_address, &lungh);

	/*acquis IP e PORT remoti*/
	if(gpn==-1)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-5600);
	}

	/* trasformo le cifre in stringhe */
	/*
	sprintf(lha1, "%d", local_address.sin_addr.s_net);
	sprintf(lha2, "%d", local_address.sin_addr.s_host);
	sprintf(lha3, "%d", local_address.sin_addr.s_lh);
	sprintf(lha4, "%d", local_address.sin_addr.s_impno);
	sprintf(sha1, "%d", remote_address.sin_addr.s_net);
	sprintf(sha2, "%d", remote_address.sin_addr.s_host);
	sprintf(sha3, "%d", remote_address.sin_addr.s_lh);
	sprintf(sha4, "%d", remote_address.sin_addr.s_impno);
	*/
	sprintf(lha1, "%d", (int)local_address.sin_addr.s_addr&0xff);
	sprintf(lha2, "%d", (int)(local_address.sin_addr.s_addr>>8)&0xff);
	sprintf(lha3, "%d", (int)(local_address.sin_addr.s_addr>>16)&0xff);
	sprintf(lha4, "%d", (int)(local_address.sin_addr.s_addr>>24)&0xff);
	sprintf(sha1, "%d", (int)remote_address.sin_addr.s_addr&0xff);
	sprintf(sha2, "%d", (int)(remote_address.sin_addr.s_addr>>8)&0xff);
	sprintf(sha3, "%d", (int)(remote_address.sin_addr.s_addr>>16)&0xff);
	sprintf(sha4, "%d", (int)(remote_address.sin_addr.s_addr>>24)&0xff);

	/* passo a riempire le str : serv_host_IPadd e local_host_IPadd*/
	strcpy(pf_local_host_IPaddr, lha1); strcat(pf_local_host_IPaddr, ",");
	strcat(pf_local_host_IPaddr, lha2); strcat(pf_local_host_IPaddr, ",");
	strcat(pf_local_host_IPaddr, lha3); strcat(pf_local_host_IPaddr, ",");
	strcat(pf_local_host_IPaddr, lha4); 

	strcpy(serv_host_IPaddr, sha1); strcat(serv_host_IPaddr, ",");   
	strcat(serv_host_IPaddr, sha2); strcat(serv_host_IPaddr, ",");   
	strcat(serv_host_IPaddr, sha3); strcat(serv_host_IPaddr, ",");   
	strcat(serv_host_IPaddr, sha4);

	/* ottengo  gli interi identificativi delle due porte */
	loc_comm_port=ntohs(local_address.sin_port);
	rem_comm_port=ntohs(remote_address.sin_port);

	/* ottengo le stringhe relative alle due porte */
	sprintf(loc_comm_port_str, "%i", loc_comm_port);
	sprintf(rem_comm_port_str, "%i", rem_comm_port);

	/* BEGINNING OF COMMUNIC ON COMMAND'S CHANNEL */

	if( (ret_code=commread(commsockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	strcpy(m1,"USER ");
	strcat(m1, pf_login);
	strcat(m1, "\r\n");
	log_len=strlen(m1);

	/*fase di login */
	if( (log_ret=FTPSendCmd(commsockfd, m1, log_len)) > 0)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-log_ret-12);
	}

	if( (ret_code=commread(commsockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	strcpy(m2,"PASS ");
	strcat(m2, pf_password);
	strcat(m2, "\r\n");
	log_len=strlen(m2);

	/*fase di password */
	if( (log_ret=FTPSendCmd(commsockfd, m2, log_len)) >0 )
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-log_ret-14);
	}

	if( (ret_code=commread(commsockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

/* rm 16-06-1999 : eliminato per problemi con AS400 */
/* GG 06-12-2000 : rimesso per problemi con Linux (!) */
#if 1
	/*setto BINARIO*/
	if( (port_ret=FTPSendCmd(commsockfd, "TYPE I\r\n", 8)) > 0 )
	{
		pcftp_errno=errno;
		closesocket(commsockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-15);
	}
#endif

	/*fclose(pf_fdnull);*/
	pcftp_errno=errno;
	return(commsockfd);
}

/*LIB*/

/*********************************************************************/
/****************** HANG_OFF *****************************************/
/*********************************************************************/

int  hang_off(int zocket)
{
	int		ret;
	char		del_command[LUNG_PATH+50]="", stringa[1024];
	int		port_ret=0, del_len=0;
	/*FILE 		*pf_fdnull;*/

	pcftp_error_class=7;
	errno=0;
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	strcpy(del_command, "QUIT");
	strcat(del_command, "\r\n" );
	del_len=strlen(del_command);
	port_ret=FTPSendCmd(zocket, del_command, del_len);
	if(port_ret>0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(zocket);
		return(-port_ret-37);
	}

	/*fclose(pf_fdnull);*/
	pcftp_errno=errno;

	errno=0;
	ret=closesocket(zocket);
	pcftp_errno=errno;
	return(ret);
}

/*LIB*/

/*********************************************************************/
/***************** REM_GET FUNCTION **********************************/
/*********************************************************************/

/*
* rm 10-06-1999 : modifica per gestione sistemi AS400
*/
int rem_get(char file[], char locdir[], int zocket)
{
	int					data_sockfd, lungh, port_len, get_len;
	int					a, b, gsn1, port_ret, ret_code, ftperr;
	int 				err;
	char				loc_data_port_str_a[8], loc_data_port_str_b[8];
	char				full_loc_data_sockpair[30], nomefile_locale[LUNG_PATH];
	char				port_command[LUNG_PATH+50], retrieve[LUNG_PATH+50];
	char				nomefile_remoto[LUNG_PATH], test[3], stringa[1024];
	int					data_newsockfd, sockfd;
	struct sockaddr_in	new_client_address;
	struct sockaddr_in	data_client_address;
	u_short				loc_data_port;
	FILE				*pf_fdget;
	/*FILE				*pf_fdnull;*/

	errno=0;
	pcftp_error_class=2;
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/
	sockfd=zocket;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	lungh=sizeof(new_client_address);
	if ( (data_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-2200);
	}

#ifdef	EXCEED
	bind_socket(data_sockfd,&new_client_address);
#endif

	listen(data_sockfd, 5);
	gsn1 = getsockname(data_sockfd,(struct sockaddr *)&new_client_address,&lungh);
	if(gsn1<0) {
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-1700);
	}
	loc_data_port=ntohs(new_client_address.sin_port);

	a=loc_data_port/256;
	b=loc_data_port-(a*256);
	sprintf(loc_data_port_str_a, "%d", a);
	sprintf(loc_data_port_str_b, "%d", b);
	strcpy(port_command, "PORT ");
	strcpy(full_loc_data_sockpair, pf_local_host_IPaddr);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_a);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_b);
	strcat(full_loc_data_sockpair, "\r\n");
	strcat(port_command, full_loc_data_sockpair);
	port_len=strlen(port_command);

	port_ret=FTPSendCmd(sockfd, port_command, port_len);       /*invio comando PORT*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-77);
	}
	if((ret_code=commread(sockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	strcpy(retrieve, "RETR ");

	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (locdir[strlen(locdir)-1]=='/') locdir[strlen(locdir)-1]=0;

	strcpy(nomefile_locale, locdir);
	strcat(nomefile_locale, "/");
	strcat(nomefile_locale, file);

	strcpy(nomefile_remoto, file);
	strcat(retrieve, nomefile_remoto);
	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);

	port_ret=FTPSendCmd(sockfd, retrieve, get_len);         /*invio comando RETR*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK);  		/*testo eventuali errori*/
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if(  test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}


	pf_fdget=fopen(nomefile_locale, "w");
	if(pf_fdget==0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-1800);
	}
	data_newsockfd=accept(data_sockfd, (struct sockaddr *)&data_client_address,&lungh);
	closesocket(data_sockfd);
	if(data_newsockfd<=0)
	{
		pcftp_errno=errno;
		fclose(pf_fdget);
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-2000);
	}
	if( fcntl(data_newsockfd, F_SETFL, O_NONBLOCK) < 0 )
	{
		pcftp_errno=errno;
		fclose(pf_fdget);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-7600);
	}

	if( (ret_code=commread(sockfd))>0)    /*aspetto il segnale 150*/
	{
		pcftp_errno=errno;
		fclose(pf_fdget);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}

	/*aspetto la copia*/
	if( (ret_code=dataread(data_newsockfd, pf_buffer, pf_fdget))>0)
	{
		pcftp_errno=errno;
		fclose(pf_fdget);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code-9);
	}
	if( (ret_code=commread(sockfd))>0)     /*aspetto il segnale 226*/
	{
		pcftp_errno=errno;
		fclose(pf_fdget);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}
	pcftp_errno=errno;
	fclose(pf_fdget);
	closesocket(data_newsockfd);
	/*fclose(pf_fdnull);*/
	return(0);
}

/*LIB*/

/*****************************************************************/
/***************** REM_PUT FUNCTION ******************************/
/*****************************************************************/

/*
* rm 10-06-1999 : modifica per gestione sistemi AS400
*/
int rem_put(char *file, char *locdir, int zocket)
{
	int					data_sockfd, lungh, port_len, put_len;
	int					a, b, gsn1, port_ret, ret_code, ftperr/*, rr*/;
	int					data_newsockfd, wf_ret, sockfd;
	char				loc_data_port_str_a[8], loc_data_port_str_b[8];
	char				port_command[LUNG_PATH+50], store[LUNG_PATH+50], test[3];
	char				full_loc_data_sockpair[50];
	char				nomefile_remoto[LUNG_PATH];
	char				nomefile_locale[LUNG_PATH], stringa[1024];
	struct sockaddr_in	new_client_address, data_client_address;
	u_short				loc_data_port;
	FILE 				*pf_fdput;
	/*FILE 				*pf_fdnull;*/

	errno=0;
	pcftp_error_class=1;

	sockfd=zocket;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/

	lungh=sizeof(new_client_address);
	if ( (data_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-2200);
	}

#ifdef	EXCEED
	bind_socket(data_sockfd,&new_client_address);
#endif

	listen(data_sockfd, 5);
	gsn1 = getsockname(data_sockfd,
	                   (struct sockaddr *) &new_client_address,  &lungh);
	if(gsn1<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-1700);
	}
	loc_data_port=ntohs(new_client_address.sin_port);

	a=loc_data_port/256;
	b=loc_data_port-(a*256);
	sprintf(loc_data_port_str_a, "%d", a);
	sprintf(loc_data_port_str_b, "%d", b);
	strcpy(port_command, "PORT ");
	strcpy(full_loc_data_sockpair, pf_local_host_IPaddr);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_a);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_b);
	strcat(full_loc_data_sockpair, "\r\n");
	strcat(port_command, full_loc_data_sockpair);
	port_len=strlen(port_command);

	port_ret=FTPSendCmd(sockfd, port_command, port_len);	/*invio comando PORT*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-77);
	}
	if( (ret_code=commread(sockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}


	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (locdir[strlen(locdir)-1]=='/') locdir[strlen(locdir)-1]=0;

	strcpy(nomefile_locale, locdir);
	strcat(nomefile_locale, "/");
	strcat(nomefile_locale, file);
	pf_fdput=fopen(nomefile_locale, "r");
	if(pf_fdput==0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-1800);
	}

	strcpy(store, "STOR ");
	strcpy(nomefile_remoto, file);
	strcat(store, nomefile_remoto);
	strcat(store, "\r\n" );
	put_len=strlen(store);

	port_ret=FTPSendCmd(sockfd, store, put_len);		/*invio comando STOR*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		fclose(pf_fdput);
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK);  		/*testo eventuali errori*/
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}

	data_newsockfd =accept(data_sockfd, (struct sockaddr *)&data_client_address, &lungh);
	closesocket(data_sockfd);
	if(data_newsockfd<=0)
	{
		pcftp_errno=errno;
		fclose(pf_fdput);
		/*fclose(pf_fdnull);*/
		return(-2000);
	}
	if( (ret_code=commread(sockfd))>0)	/*aspetto il segnale 150*/
	{
		pcftp_errno=errno;
		fclose(pf_fdput);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}
	wf_ret=writefile(data_newsockfd, pf_buffer, pf_fdput);		/*aspetto la copia*/
	if(wf_ret>0)
	{
		/*rr=recv(sockfd, stringa, 1000, 0);*//*pulizia del canale-comandi*/
		pcftp_errno=errno;
		fclose(pf_fdput);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-wf_ret-9);
	}
	if( (ret_code=commread(sockfd))>0)	/*aspetto il segnale 226*/
	{
		pcftp_errno=errno;
		fclose(pf_fdput);
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}
	pcftp_errno=errno;
	fclose(pf_fdput);
	/*fclose(pf_fdnull);*/
	return(0);
}

/*LIB*/

/************************************************************************/
/***************** REM_DIR2 FUNCTION ************************************/
/************************************************************************/

int rem_dir2(char remdir[], FILE *out, int zocket)
{
	int   data_sockfd, lungh, port_len, get_len, pp, ss;
	int   a, b, gsn1, port_ret, ret_code, bb, dedonno,cc;
	int   data_newsockfd, sockfd, ftperr;
	char  loc_data_port_str_a[8], loc_data_port_str_b[8];
	char  port_command[LUNG_PATH+50], retrieve[LUNG_PATH+50], stringa[1024];
	char  test[3], full_loc_data_sockpair[50], directoryname[LUNG_PATH+50];
	char  nomefile_remoto[LUNG_PATH], ff, filename[LUNG_PATH];
	struct sockaddr_in new_client_address, data_client_address;
	u_short loc_data_port;


	errno=0;
	pcftp_error_class=4;
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/

	sockfd=zocket;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	lungh=sizeof(new_client_address);
	if ((data_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-2200);
	}

#ifdef  EXCEED
	bind_socket(data_sockfd,&new_client_address);
#endif

	listen(data_sockfd, 5);
	gsn1 = getsockname(data_sockfd, (struct sockaddr *) &new_client_address,  &lungh);
	if(gsn1<0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-1700);
	}
	loc_data_port=ntohs(new_client_address.sin_port);

	a=loc_data_port/256;
	b=loc_data_port-(a*256);
	sprintf(loc_data_port_str_a, "%d", a);
	sprintf(loc_data_port_str_b, "%d", b);
	strcpy(port_command, "PORT ");
	strcpy(full_loc_data_sockpair, pf_local_host_IPaddr);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_a);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_b);
	strcat(full_loc_data_sockpair, "\r\n");
	strcat(port_command, full_loc_data_sockpair);
	port_len=strlen(port_command);

	port_ret=FTPSendCmd(sockfd, port_command, port_len);       /*invio comando PORT*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-77);
	}
	if( (ret_code=commread(sockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	dedonno=cc=0;
	pp=strlen(remdir);
	ff=remdir[pp-1];
	if( ff=='/' )
	{
		dedonno=0;
		strcpy(retrieve, "CWD ");
		strcpy(nomefile_remoto, remdir);
		strcat(retrieve, nomefile_remoto);
	}
	else
	{
		dedonno=1;
		ss=pp-1;
		while( remdir[ss]!='/' )
			ss--;
		for( bb=0; bb<=ss; bb++ )
			directoryname[bb]=remdir[bb];
		directoryname[bb]='\0';
		for( bb=ss+1; bb<pp; bb++ )
		{
			filename[cc]=remdir[bb];
			cc++;
		}
		filename[cc]='\0';
		strcpy(retrieve, "CWD ");
		strcpy(nomefile_remoto, directoryname);
		strcat(retrieve, nomefile_remoto);
	}




	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (retrieve[strlen(retrieve)-1]=='/') retrieve[strlen(retrieve)-1]=0;

	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);

	port_ret=FTPSendCmd(sockfd, retrieve, get_len);      /*invio comando CWD*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK);              /* testo eventuali errori */
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}

	if( (ret_code=commread(sockfd))>0)      /*aspetto il segnale 250*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	if( !dedonno )
	{
		strcpy(retrieve, "LIST");
	}
	if( dedonno )
	{
		strcpy(retrieve, "LIST ");
		strcat(retrieve, filename);
	}

	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);

	port_ret=FTPSendCmd(sockfd, retrieve, get_len);      /*invio comando LIST*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK);              /* testo eventuali errori */
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}
	data_newsockfd =accept(data_sockfd,     (struct sockaddr *)&data_client_address, &lungh);

	closesocket(data_sockfd);
	if(data_newsockfd<=0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-2000);
	}
	if( fcntl(data_newsockfd, F_SETFL, O_NONBLOCK) < 0 )
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-7600);
	}

	if( (ret_code=commread(sockfd))>0)      /*aspetto il segnale 150*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}

	if( (ret_code=directory_read(data_newsockfd, pf_buffer, out))>0)        /*aspetto la copia*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code-9);
	}
	if( (ret_code=commread(sockfd))>0)      /*aspetto il segnale 226*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}
	pcftp_errno=errno;
	closesocket(data_newsockfd);
	/*fclose(pf_fdnull);*/
	return(0);
}

/*LIB*/

/************************************************************************/
/***************** REM_DIR FUNCTION *************************************/
/************************************************************************/

int rem_dir(char remdir[], FILE *out, int zocket)
{
	int					data_sockfd, lungh, port_len, get_len;
	int					a, b, gsn1, port_ret, ret_code;
	int					data_newsockfd, sockfd, ftperr;
	char				loc_data_port_str_a[8], loc_data_port_str_b[8];
	char				port_command[LUNG_PATH+50], retrieve[LUNG_PATH+50], stringa[1024];
	char				test[3], full_loc_data_sockpair[50];
	char				nomefile_remoto[LUNG_PATH];
	struct sockaddr_in	new_client_address, data_client_address;
	u_short				loc_data_port;
	/*FILE 				*pf_fdnull;*/


	errno=0;
	pcftp_error_class=4;
	recv(zocket, stringa, 100, 0);/*pulizia del canale-comandi*/

	sockfd=zocket;
	/*pf_fdnull=fopen(NO_OUT,"w");*/
	lungh=sizeof(new_client_address);
	if ((data_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-2200);
	}

#ifdef	EXCEED
	bind_socket(data_sockfd,&new_client_address);
#endif

	listen(data_sockfd, 5);
	gsn1 = getsockname(data_sockfd, (struct sockaddr *) &new_client_address,  &lungh);
	if(gsn1<0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-1700);
	}
	loc_data_port=ntohs(new_client_address.sin_port);

	a=loc_data_port/256;
	b=loc_data_port-(a*256);
	sprintf(loc_data_port_str_a, "%d", a);
	sprintf(loc_data_port_str_b, "%d", b);
	strcpy(port_command, "PORT ");
	strcpy(full_loc_data_sockpair, pf_local_host_IPaddr);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_a);
	strcat(full_loc_data_sockpair, ",");
	strcat(full_loc_data_sockpair, loc_data_port_str_b);
	strcat(full_loc_data_sockpair, "\r\n");
	strcat(port_command, full_loc_data_sockpair);
	port_len=strlen(port_command);

	port_ret=FTPSendCmd(sockfd, port_command, port_len);       /*invio comando PORT*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-77);
	}
	if( (ret_code=commread(sockfd))>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	strcpy(retrieve, "CWD ");
	strcpy(nomefile_remoto, remdir);

	strcat(retrieve, nomefile_remoto);

	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	if (retrieve[strlen(retrieve)-1]=='/') retrieve[strlen(retrieve)-1]=0;

	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);

	port_ret=FTPSendCmd(sockfd, retrieve, get_len);      /*invio comando CWD*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK); 		 /* testo eventuali errori */
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}

	if( (ret_code=commread(sockfd))>0)    	/*aspetto il segnale 250*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		return(-ret_code);
	}

	strcpy(retrieve, "LIST");
	strcpy(nomefile_remoto, remdir);

	strcat(retrieve, "\r\n" );
	get_len=strlen(retrieve);

	port_ret=FTPSendCmd(sockfd, retrieve, get_len);      /*invio comando LIST*/
	if(port_ret>0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-port_ret-21);
	}
	port_ret=nnrecv(sockfd, test, 3, MSG_PEEK); 		 /* testo eventuali errori */
	if(port_ret<0)
	{
		pcftp_errno=errno;
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-7800);
	}
	if( test[0]=='5' )
	{
		pcftp_errno=errno;
		ftperr=atoi(test);
		closesocket(data_sockfd);
		/*fclose(pf_fdnull);*/
		return(-3000-ftperr);
	}
	if( test[0]=='4' )
	{
		/*fclose(pf_fdnull);*/
		return(9999);
	}
	data_newsockfd =accept(data_sockfd,	(struct sockaddr *)&data_client_address, &lungh);

	closesocket(data_sockfd);
	if(data_newsockfd<=0)
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_sockfd);
		return(-2000);
	}
	if( fcntl(data_newsockfd, F_SETFL, O_NONBLOCK) < 0 )
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-7600);
	}

	if( (ret_code=commread(sockfd))>0)    	/*aspetto il segnale 150*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}

	if( (ret_code=directory_read(data_newsockfd, pf_buffer, out))>0) 	/*aspetto la copia*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code-9);
	}

	if( (ret_code=commread(sockfd))>0)     	/*aspetto il segnale 226*/
	{
		pcftp_errno=errno;
		/*fclose(pf_fdnull);*/
		closesocket(data_newsockfd);
		return(-ret_code);
	}
	pcftp_errno=errno;
	closesocket(data_newsockfd);
	/*fclose(pf_fdnull);*/
	return(0);
}

/*LIB*/

/************************************************************************/
/***************** RET_INF_PCFTP *************************************/
/************************************************************************/

int ret_inf_pcftp(int tipoinf)
{
	if (tipoinf==INF_PCFTP_ERRNO)
		return(pcftp_errno);
	else if(tipoinf==INF_PCFTP_CLASS)
		return(pcftp_error_class);
	else
		return(0);
}

/*LIB*/

/*******************************************************************/
/********************* COMMREAD (7000) (7400)***********************/
/*******************************************************************/

int commread(int s)
{
	int	r=0, val=0, nChar=0,nMoreLines=0;
	char cRcvdChar, x2[128]="";
	char szRcvdString[256];

	helpflag=0;

	while(1) {
		r=nnrecv(s, &cRcvdChar, 1, 0);
		if(r<0)
		{
			return(7400);
		}
		szRcvdString[nChar]=cRcvdChar;

		if(cRcvdChar=='5'&&nChar==0)
		{
			r=nnrecv(s, x2, 2, 0);
			val=500+atoi(x2);
		}
		if(cRcvdChar=='4'&&nChar==0)
		{
			r=nnrecv(s, x2, 2, 0);
			val=400+atoi(x2);
		}

		if(cRcvdChar=='-' && nChar == 3){
			/*
			* messaggio su piu` righe
			*/
			nMoreLines=1;

		}

		if( cRcvdChar=='\n' ){
			if(!nMoreLines){
				szRcvdString[nChar]='\0';
				break;
			} else {
				/*
				* resetto il flag di altre linee
				*/
				nMoreLines=0;
			}
		}

		nChar++;
	}
#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,"<<< %s\n",szRcvdString);
		fclose(fpDebug);
	}
#endif
	return(val);
}

/*LIB*/

/*********************************************************************/
/********************* DATAREAD (7500) (7300)  (7900)? ***************/
/*********************************************************************/

int dataread(int s, char buffer[], FILE *fdg)
{
	int k=0, countbuffer=0, nw=0, nw2=0, count=0;
	char x;

	while(1)
	{
		helpflag=1;
		nw2=nnrecv(s, &x, 1, MSG_PEEK);
		if(nw2<=0)
			break;
		helpflag=1;
		nw=nnrecv(s, buffer, BUFFERSIZE, 0);
		if(nw <= 0)
			return(7500);
		if(nw==BUFFERSIZE)
			countbuffer++;
		count=nw;
		k=fwrite(buffer, 1, nw, fdg);
		if(k==-1 || k==0)
			return(7300);
	}
	return(0);
}

/*LIB*/

/******************************************************************/
/********************* DIRECTORY_READ (7100) (7200) ***************/
/******************************************************************/

int directory_read(int s, char buffer[], FILE *out)
{
	int k=0, countbuffer=0, nw=0, nw2=0, count=0;
	char x;
	/* SB Modifica per utilizzare il Server FTP di EXCEED */
	char  appoggio[BUFFERSIZE];
	int conta,posizione;

	while(1)
	{
		helpflag=1;
		nw2=nnrecv(s, &x, 1, MSG_PEEK);
		if(nw2<=0)
			break;
		helpflag=1;
		nw=nnrecv(s, buffer, BUFFERSIZE, 0);
		if(nw <= 0)
			return(7100);
		if(nw==BUFFERSIZE)
			countbuffer++;
		count=nw;
		/* SB Modifica per utilizzare il Server FTP di EXCEED */
		posizione=0;
		for (conta=0;conta<nw;conta++)
			if (buffer[conta]!='\r')
			{
				appoggio[posizione]=buffer[conta];
				posizione++;
			}

		k=fwrite(appoggio, 1,posizione, out);
		/*	k=fwrite(buffer, 1, nw, out);*/
		if(k==-1 || k==0)
			return(7200);
	}

	return(0);
}

/*LIB*/

/********************************************************************/
/************************** WRITEFILE (7800)*************************/
/********************************************************************/

int writefile(int sockfd, char buffer[], FILE *fdp)
{
	int nw=0, k=0, buffcount=0, count=0;

	while(1)
	{
		k=fread(buffer, 1, BUFFERSIZE, fdp);
		if( k==BUFFERSIZE )
			buffcount++;
		if(k==-1||k==0)
			break;
		count=k;
		/*nw=send(sockfd, buffer, k, 0);*/
		nw=writen(sockfd, buffer, k);
		if(nw<0)
		{
			closesocket(sockfd);
			return(7800);
		}
	}
	closesocket(sockfd);
	return(0);
}


/*****************************************************************/
/************************** WRITEN (7700)*************************/
/*****************************************************************/

int writen(register int fd, register char *ptr, register int nbytes)
{
	int nleft, nwritten;

	nleft=nbytes;
	while(nleft>0)
	{
		nwritten=send(fd, ptr, nleft, 0);
		if(nwritten<0 )
			return(7700);
		if(nwritten>0)
		{
			nleft-=nwritten;
			ptr+=nwritten;
		}
	}
	return(0);
}

/*****************************************************************/
/************************** FTPSendCmd       *********************/
/*****************************************************************/

int FTPSendCmd(register int fd, register char *ptr, register int nbytes)
{
	int nleft, nwritten;

#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"a+"))!=(FILE *)NULL){
		fprintf(fpDebug,">>> %s",ptr);
		fclose(fpDebug);
	}
#endif
	nleft=nbytes;
	while(nleft>0)
	{
		nwritten=send(fd, ptr, nleft, 0);
		if(nwritten<0 )
			return(7700);
		if(nwritten>0)
		{
			nleft-=nwritten;
			ptr+=nwritten;
		}
	}
	return(0);
}



/*******************************************************/
/***************** NRECV *******************************/
/*******************************************************/
int nnrecv(int sock, char stringa[], int numb, int flag)
{
	int	nw=0;
	double	tempo=0, tempot=0;

	while(1)
	{
		tempo=aspetta();
		nw=-1;  errno=11;
		while( errno==11 && nw<0 )
		{
			tempot=aspetta();
			if( (tempot-tempo)>DEFUNCT2 ) /*qui attende sul canale dati=>gli concedo un tempo maggiore*/
			{
				fprintf(stderr,"scaduto timeout di %d secondi : errno=%d\n", DEFUNCT2/1000000, errno);
				shutdown(sock, 2);
				helpflag=0;
				return(-1);
			}
			if( (tempot-tempo)>DEFUNCT ) /*qui attende sul canale controllo=>gli concedo un tempo minore*/
			{
				if( helpflag==0 )
				{
					fprintf(stderr,"scaduto timeout di %d secondi : errno=%d\n", DEFUNCT/1000000, errno);
					shutdown(sock, 2);
					helpflag=0;
					return(-1);
				}
			}
			errno=0;
			nw=recv(sock, &stringa[0], numb, flag);
		}
		break;
	}
	if(nw<0)
	{
		fprintf(stderr,"problema grosso grosso: errno=%d\n", errno);
		shutdown(sock, 2);
		helpflag=0;
		return(-1);
	}
	helpflag=0;
	return(nw);
}

/*LIB*/

/********************************************************************/
/************************** BIND_SOCKET solo EXCEED******************/
/********************************************************************/

#ifdef EXCEED
int	bind_socket(SOCKET s,LPSOCKADDR_IN address)
{
	memset((char *) address, '\0', sizeof(SOCKADDR_IN));

	address->sin_family=AF_INET;
	address->sin_addr.s_addr=INADDR_ANY;
	address->sin_port=0;

	return(bind(s, (LPSOCKADDR) address, sizeof(SOCKADDR_IN)));
}
#endif


/**************************************************/
/*************  ASPETTA  **************************/
/**************************************************/
double aspetta(void)
{
	long microsecondi=0, secondi=0;
	long double adesso=0;
	struct timeval ora;
	struct timezone tipo;
	gettimeofday(&ora, &tipo);

	microsecondi=ora.tv_usec;
	secondi=ora.tv_sec;

	adesso=(double)microsecondi+(secondi*1000000.0);

	return(adesso);
}



/*
* DA FARE : funzione di gestione risposta FTP server
*/
char *CollectLineAnswer(int fd,char *szLine)
{
}
int ftp_reset_debug(void)
{
#ifdef DEBUG
	if((fpDebug=fopen(szDebugFileName,"w+"))!=(FILE *)NULL){
		fprintf(fpDebug,"GOT RESET\n");
		fclose(fpDebug);
	}
#endif
}
