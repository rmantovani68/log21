/*
* send_file.c
* Esempio di spedizione di file via FTP
*/
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"libpcftp.h"

int main (int argc, char **argv)
{
	int 			err=0;
	int				sock;

	if (argc!=7) {
		printf ("%s <file> <dir remoto> <dir locale> <host> <user> <passwd> \n",argv[0]);
		printf("   Legge il file dal <dir remoto> di <host>\n");
		printf("   e lo deposita nel <dir locale> \n");
		printf("   esempio: %s file remote_dir local_dir host user passwd\n",argv[0]);
		exit(1);
	}

	if((sock=hook_up(argv[4],argv[5],argv[6]))<0) {
		printf("Errore PCFTP: errore=%d classe=%d errno=%d\n",
			RET_CODE_PCFTP(err),
			ret_inf_pcftp(INF_PCFTP_CLASS),
			ret_inf_pcftp(INF_PCFTP_ERRNO)); 
		exit(1);
	}
	if((err=rem_put(argv[1], argv[2], argv[3],sock))!=0) {
		printf("Errore PCFTP: errore=%d classe=%d errno=%d\n",
			RET_CODE_PCFTP(err),
			ret_inf_pcftp(INF_PCFTP_CLASS),
			ret_inf_pcftp(INF_PCFTP_ERRNO)); 
		exit(1);
	}
	if(hang_off(sock)<0) {
		printf("Errore PCFTP: errore=%d classe=%d errno=%d\n",
			RET_CODE_PCFTP(err),
			ret_inf_pcftp(INF_PCFTP_CLASS),
			ret_inf_pcftp(INF_PCFTP_ERRNO)); 
		exit(1);
	}
	exit(0);
}
