/* 
* Modulo : recv_file.c
* ------------------
* Ricezione di un file via FTP
*
* Utilizzo : recv_file <file> <dir remoto> <dir locale> <host> <user> <passwd>
*
* Autore : Roberto Mantovani
*
* Copyright CNI srl 1999
*/
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"libpcftp.h"

int main (int argc, char **argv)
{
	int 			err=0;

	if (argc!=7) {
		printf ("%s <file> <dir remoto> <dir locale> <host> <user> <passwd> \n",argv[0]);
		printf("   Legge il file dal <dir remoto> di <host>\n");
		printf("   e lo deposita nel <dir locale> \n");
		printf("   esempio: %s file remote_dir local_dir host user passwd\n",argv[0]);
		exit(1);
	}
	err=receive_file(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
	if(err>0){
		printf("Errore : %d\n",err);
	}

	exit(0);
}
