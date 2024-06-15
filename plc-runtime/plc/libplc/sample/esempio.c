
#include <stdio.h>

#include <nvram.h>
#include <plclink.h>

TRACERECORD buffer[400];

void main(void)
{
char *p,*q;
int i,j,rv,n;
float f;
long x,l,tcurr,tmin,tmax;
char name[200];
struct plcvar_t t;
struct modprop_t prop;
nvram_t d;

	plcOpen();

	if (AttachNvram(NVRAM_LBL_PLC480,&d,NULL,0) >= 0) {
		dbRegisterNvram(d.addr);
	}

	for (;;) {
		name[0] = '\0';
		plcGetExeName(name);
		printf("\nEseguibile PLC : %s\n", name);
		printf("\nStato plc : %s\n\n", plcStatus() ? "go" : "halt");
		printf("\
1 - lettura di 4 byte dalla memoria del PLC\n\
2 - start plc\n3 - stop plc\n\
4 - lettura di una variabile\n\
5 - scrittura di una variabile\n\
6 - visualizzazione delle caratteristiche di una variabile\n\
7 - uscita\n8 - test di trace\n\
9 - carica un oggetto eseguibile\n\
10 - scarica un oggetto eseguibile\n\
11 - statistica\n\
12 - azzeramento statistica\n\
13 - informazioni sui moduli caricati\n");
		scanf("%d",&n);
		switch (n) {
		case 1:
			printf("Indirizzo esadecimale ? (vedi \"nm isaker\")\n");
			scanf(" %lx",&x);
			rv = plcMemCopy((char *)&l,(char *)x,sizeof(l));
			if (rv) {
				printf("plcMemCopy : Errore %d\n",rv);
				break;
			}
			printf("0x%08lx --> 0x%08lx\n",x,l);
			break;
		case 2:
			if (plcGo())
				printf("Plc avviato.\n");
			else
				printf("Qualcosa e` andato storto.\n");
			break;
		case 3:
			if (plcHalt())
				printf("Plc fermo.\n");
			else
				printf("Qualcosa e` andato storto.\n");
			break;
		case 4:
			printf("Variabile ?\n");
			scanf(" %s",name);
			if (! dbGetVar(name,&t))
				printf("\"%s\" non esiste !\n",name);
			else {
				switch (t.type) {
				case ISAVAR_T_BOOL:
					printf("%s = %s\n",name,*(char *)t.pval ? t.format[1] : t.format[0]);
					break;
				case ISAVAR_T_ANA_I:
				case ISAVAR_T_TIMER:
					printf("%s = %ld\n",name,*(long *)t.pval);
					break;
				case ISAVAR_T_ANA_F:
					printf("%s = %f\n",name,*(float *)t.pval);
					break;
				default:
					printf("Tipo sconosciuto !\n");
					break;
				}
			}
			break;
		case 5:
			printf("Variabile ?\n");
			scanf(" %s",name);
			if (! dbGetVar(name,&t))
				printf("\"%s\" non esiste !\n",name);
			else {
				switch (t.type) {
				case ISAVAR_T_BOOL:
					printf("Valore (BOOL) ?\n");
					scanf("%d",&i);
					*(char *)t.pval = (char)(i ? 1 : 0);
					break;
				case ISAVAR_T_ANA_I:
				case ISAVAR_T_TIMER:
					printf("Valore (INT) ?\n");
					scanf("%ld",&l);
					*(long *)t.pval = l;
					break;
				case ISAVAR_T_ANA_F:
					printf("Valore (FLOAT) ?\n");
					scanf("%f",&f);
					*(float *)t.pval = f;
					break;
				default:
					printf("Tipo sconosciuto !\n");
					break;
				}
			}
			break;
		case 6:
			printf("Variabile ?\n");
			scanf(" %s",name);
			if (! dbGetVar(name,&t))
				printf("\"%s\" non esiste !\n",name);
			else {
				printf("\nName = \"%s\"\n",t.name);
				printf("Size = %d\n",t.size);
				printf("Scope = 0x%04x\n",t.scope);
				printf("Type = %d\n",t.type);
				printf("Attr = %d\n",t.attr);
				printf("Format = \"%s\", \"%s\"\n\n",t.format[0],t.format[1]);
			}
			break;
		case 7:
			exit(0);
		case 8:
			printf("Variabile ?\n");
			scanf(" %s",name);
			rv = plcTraceVar(name);
			if (rv) {
				printf("plcTraceVar : Errore %d\n",rv);
				break;
			}
			rv = plcTraceStart();
			if (rv) {
				printf("plcTraceStart : Errore %d\n",rv);
				break;
			}
			for (i = 0; i < 4;) {
				rv = plcTraceDownLoad(buffer);
				if (rv) {
					if (rv != PLCERR_NOTREADY) {
						printf("plcTraceDownLoad : Errore %d\n",rv);
						break;
					}
					printf(".");fflush(stdout);
					sleep(1);
				}
				else {
					printf(" %d\n",i);
					for (j = 0; j < 400; ++j)
						putchar('0' + (buffer[j] & 1));
					printf("\n");
					++i;
				}
			}
			rv = plcTraceStop();
			if (rv) {
				printf("plcTraceStop : Errore %d\n",rv);
				break;
			}
			break;
		case 9:
			printf("File ?\n");
			scanf(" %s",name);
			rv = plcLoadProg(name);
			if (rv == PLCERR_TIMEOUT) {
				while (plcLdStatus(&rv) == PLCERR_TIMEOUT) {
					printf(".");fflush(stdout);
					sleep(1);
				}
				if (rv)
					printf("\nErrore %d (0x%08x)\n",rv,rv);
				else
					printf("fatto\n");
			}
			else if (rv) {
				printf("plcLoadProg : Errore %d\n",rv);
			}
			break;
		case 10:
			rv = plcUnloadProg();
			if (rv) {
				printf("plcUnloadProg : Errore %d\n",rv);
			}
			break;
		case 11:
			rv = plcStatistics(&tcurr,&tmin,&tmax);
			if (rv) {
				printf("plcStatistics : Errore %d\n",rv);
				break;
			}
			printf("Statistica del tempo di ciclo :\n corrente = %ld\n minimo   = %ld\n massimo  = %ld\n",tcurr,tmin,tmax);
			break;
		case 12:
			rv = plcStClear();
			if (rv) {
				printf("plcStClear : Errore %d\n",rv);
				break;
			}
			printf("Statistica azzerata.\n");
			break;
		case 13:
			printf("Tipo di modulo ?\n");
			scanf(" %s",name);
			rv = plcModProp(&prop,name);
			if (rv) {
				printf("plcModProp : Errore %d\n",rv);
				break;
			}
			printf("Modulo %s :\n",name);
			printf("File %s\nLunghezza %ld\nIndirizzi :\n\ttext   = 0x%08lx\n\tdata   = 0x%08lx\n\tbss    = 0x%08lx\n\tcommon = 0x%08lx\n",
				prop.file,prop.size,(long)prop.text,(long)prop.data,(long)prop.bss,(long)prop.common);
			break;
		default:
			printf("Mbe` ?\n");
			break;
		}
	}
}

