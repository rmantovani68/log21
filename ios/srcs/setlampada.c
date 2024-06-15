/*
* setlampada.c : gestione IOS
* 
* Progetto Mondadori Picking
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <hwcni.h>


#ifndef BYTE_TYPE_DEFINED
typedef unsigned char BYTE;
#define BYTE_TYPE_DEFINED
#endif

#define MAX_IOS 4
#define PKL_NULL_CMD  0x00
#define PKL_LIGHT_ON  0x01
#define PKL_LIGHT_OFF 0x02
#define PKL_RESET        0x03
#define PKL_TEST        0x04
#define PKL_FREEZE      0x05

#define PKL_BUTTON_PRESSED  0x40
#define PKL_LIGHT_DAMAGED   0x41

#define IOS_TX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(n_byte))
#define IOS_RX_BYTE(ios_n,cpu_n,mod_n,n_byte)     *((IOSInfo[(ios_n)].cpu[(cpu_n)])+(mod_n)*8+(0x300+(n_byte)))

struct ios_info_t IOSInfo[MAX_IOS];
int nFdIOS[MAX_IOS];
int nDelay=100000;

/*
* WaitIOS()
* attendo che il codice comando in TX del modulo 
* e della IOS passate in ingresso sia libero (0)
*/
void WaitIOS(int nIOS,int nCPU,int nModulo,int nDelay)
{
    while(IOS_TX_BYTE(nIOS,nCPU,nModulo,0)){
        usleep(nDelay);
    }
}
/* 
* main() 
*/
void main(int argc, char **argv)
{
  unsigned char *p;
    int fd;
    int nRC;
    int nIOS;
    int nCPU;
    int nModulo;
    int nRowIndex;
    int nColIndex;
    int bExit=0;


    if (argc != 6) {
        printf("Utilizzo : %s <IOS> <CPU> <Modulo> <Riga> <Colonna>\n",argv[0]);
        return;
    } else {
        nIOS=atoi(argv[1]);
        nCPU=atoi(argv[2]);
        nModulo=atoi(argv[3]);
        nRowIndex=atoi(argv[4]);
        nColIndex=atoi(argv[5]);
    }


    nRC=iosOpenGeneral(0,&IOSInfo[0],&nFdIOS[0]);


    /*
    * gestire il valore di ritorno ....
    */
    printf("Versione IOS %d %c%c%c%c\n", 
        nIOS,
        IOSInfo[0].base[0x225],
        IOSInfo[0].base[0x226],
        IOSInfo[0].base[0x227],
        IOSInfo[0].base[0x228]);

    WaitIOS(nIOS,nCPU,nModulo,nDelay);

    IOS_TX_BYTE(nIOS,nCPU,nModulo,1)=(BYTE)0;              /* azione */
    IOS_TX_BYTE(nIOS,nCPU,nModulo,0)=(BYTE)PKL_FREEZE;     /* comando */

    WaitIOS(nIOS,nCPU,nModulo,nDelay);

    IOS_TX_BYTE(nIOS,nCPU,nModulo,1)=(BYTE)nColIndex;  /* riga */
    IOS_TX_BYTE(nIOS,nCPU,nModulo,2)=(BYTE)nRowIndex;  /* colonna */
    IOS_TX_BYTE(nIOS,nCPU,nModulo,0)=(BYTE)PKL_LIGHT_ON;   /* comando */

    WaitIOS(nIOS,nCPU,nModulo,nDelay);

    while(!bExit){
        /*
        * controllo gli eventi della IOS (tastiera o errore su lampada)
        * Byte 0 in ricezione
        */
        switch(IOS_RX_BYTE(nIOS,nCPU,nModulo,0)){
            case PKL_BUTTON_PRESSED:
                nColIndex=IOS_RX_BYTE(nIOS,nCPU,nModulo,1);
                nRowIndex=IOS_RX_BYTE(nIOS,nCPU,nModulo,2);

                printf("PKL_BUTTON_PRESSED [%2d,%2d]\n", nRowIndex,nColIndex);

                /* reset comando */
                IOS_RX_BYTE(nIOS,nCPU,nModulo,0)=0x0;

                /*
                * attendo che il codice comando in TX sia libero (0)
                */
                WaitIOS(nIOS,nCPU,nModulo,nDelay);

                /* 
                * replica del comando in TX 
                */
                IOS_TX_BYTE(nIOS,nCPU,nModulo,0)=PKL_BUTTON_PRESSED;    

                bExit=1;
                
            break;

            case PKL_LIGHT_DAMAGED:
                nColIndex=IOS_RX_BYTE(nIOS,nCPU,nModulo,1);
                nRowIndex=IOS_RX_BYTE(nIOS,nCPU,nModulo,2);

                printf("PKL_LIGHT_DAMAGED [%2d,%2d]\n", nRowIndex,nColIndex);

                /* reset comando */
                IOS_RX_BYTE(nIOS,nCPU,nModulo,0)=0x0;

                /*
                * attendo che il codice comando in TX sia libero (0)
                */
                WaitIOS(nIOS,nCPU,nModulo,nDelay);
                /* 
                * replica del comando in TX 
                */
                IOS_TX_BYTE(nIOS,nCPU,nModulo,0)=PKL_LIGHT_DAMAGED;    

                bExit=1;

            break;
            case PKL_NULL_CMD:
                /*
                * nn fare niente ...
                */
            break;
        }
        usleep(nDelay);
    }     /* fine ciclo */
}
