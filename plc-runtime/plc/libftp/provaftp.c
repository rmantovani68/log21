#include    <stdio.h>
#include    <curses.h>
#include    <string.h>
#include    <stdlib.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    "libpcftp.h"

int main (void)
{
	int             sock1,err=0,pluto;
	double          tempo=0;
	FILE		*file;

	file=fopen("~/.vimrc", "w");
	if((sock1=hook_up("trantor","ftp","ftp"))<0)
    {
    printf("Errore %d hook_up\n",sock1); 
    printf("Errore PCFTP: errore=%d classe=%d errno=%d\n",
        RET_CODE_PCFTP(err),
        ret_inf_pcftp(INF_PCFTP_CLASS),
        ret_inf_pcftp(INF_PCFTP_ERRNO)); 
    }
err=rem_dir2("/tmp/", stdout, sock1);
/*err=rem_dir2("/tmp/aggd.zip", stdout, sock1);*/
/*err=rem_dir("/win95/", file, sock1);*/
/*err=rem_dir("/home/gulini/", stdout, sock1);*/
if( err!=0  )
    {
    if(err!=9999)
        {
        printf("Errore PCFTP = %d\n",err); 
        }
    else
        {tempo=aspetta();  
        while(1)   if( (aspetta()-tempo)>=30 )  break; }                    
    }
hang_off(sock1);
fclose(file);
exit(0);
}
