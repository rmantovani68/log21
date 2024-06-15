/*
* @(#) tester.c 1.6 Tue Jun 17 12:05:36 MET DST 1997
*
* Programma di test/debug per "qplc".
*
* 06/12/96 GG 1.0 Prima versione.
* 16/12/96 GG 1.1 Aggiunti comandi per il debug in tempo reale.
* 17/12/96 GG 1.2 Altri comandi per il debug in tempo reale.
* 19/12/96 GG 1.3 Finalmente il debugger funziona !
* 11/03/97 GG 1.4 Erano scambiate le stringhe "%edi" e "%esi".
* 16/06/97 GG 1.5 Corretto un problemino nella lettura dei break (variabile
*             non inizializzata).
* 17/06/97 GG 1.6 La correzione... era sbagliata.
*/

#define VERSIONE "1.6"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

char *strdup(char *);

#include <readline/readline.h>

#include <nvram.h>
#include <plclink.h>

#define PROMPT_HALT "PLC (stopped) "
#define PROMPT_GO "PLC (running) "

#define MAX_MEM_BUFFER 200

nvram_t plc_nvram = { 0 };
char exe_path[MAXPATHLEN] = "";
int loading = 0;

static char *line = (char *)NULL;
char *rl_gets(void)
{
char *p;

	if (line) {
		free(line);
		line = (char *)NULL;
	}

	p = plcStatus() ? PROMPT_GO : PROMPT_HALT;
	line = readline(p);

	if (line && line[0])
		add_history(line);

	return line;
}

static int cm_exit(char *args)
{
	exit(0);
	return 0;
}

static int cm_halt(char *args)
{
	plcHalt();
	return 1;
}

static int cm_go(char *args)
{
	plcGo();
	return 1;
}

static int cm_help(char *args);

static int cm_module(char *args)
{
char *p;
struct modprop_t prop;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (plcModProp(&prop,p)) {
			printf(" %s : sconosciuto\n",p);
			return 1;
		}
		printf(" %s - ",p);
		if (prop.file[0]) {
			printf("file %s (%d bytes)\n"
			"      text=0x%08lx data=0x%08lx bss=0x%08lx common=0x%08lx\n",
			       prop.file,prop.size,
			       (long)(prop.text),(long)(prop.data),
			       (long)(prop.bss),(long)(prop.common));
		}
		else {
			printf("hard coded\n");
		}
	} while ( (p = strtok(NULL," \t")) );
	return 1;
}

static int cm_load(char *args)
{
int rv;

	if (! args || !args[0])
		return 0;
	rv = plcLoadProg(args);
	if (rv == PLCERR_TIMEOUT) {
		while (plcLdStatus(&rv) == PLCERR_TIMEOUT) {
			printf(".");
			fflush(stdout);
			sleep(1);
		}
		if (rv)
			printf("Errore %d\n",rv);
		else
			printf("fatto\n");
	}
	else if (rv) {
		printf("Errore %d\n",rv);
	}
	return 1;
}

static int cm_unload(char *args)
{
int rv;

	rv = plcUnloadProg();
	if (rv) {
		printf("Errore %d\n",rv);
	}
	return 1;
}

static int cm_setshvar(char *args)
{
char *p;
struct plcvar_t t;
char c;
int i;
long l;
float f;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (! dbGetVar(p,&t))
			printf("%s : segnale di scambio sconosciuto\n",p);
		else {
			if ( !(p = strtok(NULL," \t")) )
				return 0;
			switch (t.type) {
			case ISAVAR_T_BOOL:
				if (strcmp(p,"t") == 0
				  ||strcmp(p,"T") == 0
				  ||strcmp(p,"true") == 0
				  ||strcmp(p,"TRUE") == 0)
					i = 1;
				else if (strcmp(p,"f") == 0
				  ||strcmp(p,"F") == 0
				  ||strcmp(p,"false") == 0
				  ||strcmp(p,"FALSE") == 0)
					i = 0;
				else if (sscanf(p,"%d",&i) != 1)
					return 0;
				*(char *)t.pval = (char)(i ? 1 : 0);
				break;
			case ISAVAR_T_ANA_I:
			case ISAVAR_T_TIMER:
				if ((sscanf(p,"0x%lx%c",&l,&c) != 1)
				     && (sscanf(p,"%ld%c",&l,&c) != 1))
					return 0;
				*(long *)t.pval = l;
				break;
			case ISAVAR_T_ANA_F:
				if (sscanf(p,"%f%c",&f,&c) != 1)
					return 0;
				*(float *)t.pval = f;
				break;
			default:
				break;
			}
		}
	} while ( (p = strtok(NULL," \t")) );
	return 1;
}

static int cm_time(char *args)
{
char *p;
long tcurr,tmin,tmax;
int flag;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (strcmp(p,"clear") == 0)
			plcStClear();
		else if (strcmp(p,"on") == 0)
			plcStEnable(1);
		else if (strcmp(p,"off") == 0)
			plcStEnable(0);
		else if (strcmp(p,"show") == 0) {
			plcStStatus(&flag);
			if (flag) {
				plcStatistics(&tcurr,&tmin,&tmax);
				printf("Tempi : "
				"corrente=%ld minimo=%ld massimo=%ld\n",
					tcurr,tmin,tmax);
			}
			else {
				printf("Statistica disabilitata.\n");
			}
		}
		else
			return 0;
	} while ( (p = strtok(NULL," \t")) );

	return 1;
}

static int cm_status(char *args)
{
static char show[] = "show";

	printf("Stato del PLC : %s\n", plcStatus() ? "GO" : "HALT");
	printf("Path dell'esecutore : %s\n", exe_path);
	printf("Moduli :\n");
	cm_module("U_CODE");
	cm_module("C_CODE");
	cm_module("HATSH_CODE");
	cm_time(show);
	printf("\n");
	return 1;
}

static int cm_dump(char *args)
{
char *p;
unsigned long addr;
unsigned int size;
static unsigned char buffer[MAX_MEM_BUFFER];
int ds,rv,i;
static char fmt[8] = "%x";

	if (! args || !args[0])
		return 0;
	p = args;
	if (sscanf(p,"0x%lx",&addr) != 1 && sscanf(p,"%lu",&addr) != 1)
		return 0;
	p = strtok(NULL," \t");
	if (!p || (sscanf(p,"0x%x",&size) != 1 && sscanf(p,"%u",&size) != 1))
		return 0;
	p = strtok(NULL," \t");

/*
* Lavori in corso
*/

	if (! p)
		p = "bx";
	ds = 1;
	fmt[0] = '%';
	switch (p[0]) {
	case 'c':
	case 'b':
		ds = 1;
		break;
	case 'w':
		ds = 2;
		break;
	case 'l':
		ds = 4;
		break;
	case 'd':
		ds = 8;
		break;
	case ';':
		break;
	default:
		return 0;
	}
	switch (p[1]) {
	case 'c':
	case 'd':
	case 'u':
	case 'o':
	case 'x':
	case 'f':
	case 's':
		fmt[1] = p[1];
	case ';':
		break;
	default:
		return 0;
	}
	if (size >= sizeof(buffer)) {
		printf("La dimensione massima ammessa e` %d byte\n",
		        sizeof(buffer) - 1);
		return 1;
	}
	memset(buffer,0,sizeof(buffer));
	rv = plcMemCopy(buffer,(char *)addr,size);
	if (rv) {
		printf("L'intervallo 0x%08lx-0x%08lx non e` stato convalidato"
		       " dal PLC\n",addr,addr+size);
	}
	else {
		for (i = 0; i < size; i += ds) {
			if (i % 16 == 0)
				printf("%08lx: ",addr+i);
			if (fmt[1] == 's') {
				if (buffer[i] >= ' ' && buffer[i] < 127)
					printf("   %c",buffer[i]);
				else
					printf("\\x%02x",buffer[i]);
			}
			else if (fmt[1] == 'c') {
				if (buffer[i] >= ' ' && buffer[i] < 127)
					printf("%c",buffer[i]);
				else
					printf(".");
			}
			else {
				printf(" %02x",buffer[i]);
			}
			if (i % 16 == 15)
				printf("\n");
		}
		if (i % 16)
			printf("\n");
	}

	return 1;
}

static int cm_shvar(char *args)
{
char *p;
struct plcvar_t t;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (! dbGetVar(p,&t)) {
			printf("%s : segnale di scambio sconosciuto\n",p);
		}
		else {
			printf("%s = ",p);
			switch (t.type) {
			case ISAVAR_T_BOOL:
				printf("%s\n",
				       *(char *)t.pval ?
					t.format[1] : t.format[0]);
				break;
			case ISAVAR_T_ANA_I:
			case ISAVAR_T_TIMER:
				printf("%ld\n",*(long *)t.pval);
				break;
			case ISAVAR_T_ANA_F:
				printf("%f\n",*(float *)t.pval);
				break;
			default:
				break;
			}
		}
	} while ( (p = strtok(NULL," \t")) );

	return 1;
}

static int cm_trace(char *args)
{
char *p;
struct plcvar_t t;
int rv,count,i,j,k,len;
static int trlist_len = 0;
static char trlist[40][20] =  { { 0 } };
static TRACERECORD buffer[4000];
static TRACERECORD m;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (strcmp(p,"clear") == 0) {
			plcTraceStop();
			plcTraceClear();
			trlist_len = 0;
		}
		else if (strcmp(p,"add") == 0) {
			if (! (p = strtok(NULL," \t")) )
				return 0;
			rv = plcTraceVar(p);
			if (rv) {
				printf("Errore %d\n",rv);
			}
			else {
				strncpy(trlist[trlist_len++],p,
				        sizeof(trlist[0]));
			}
		}
		else if (strcmp(p,"list") == 0) {
			printf("Tracede variables :\n");
			for (i = 0; i < trlist_len; ++i) {
				printf("  %d %s\n",i,trlist[i]);
			}
		}
		else if (strcmp(p,"start") == 0) {
			if (! (p = strtok(NULL," \t")) )
				return 0;
			if (sscanf(p,"%d",&count) != 1 || count <= 0)
				return 0;
			plcTraceSetBuf(50);
			plcTraceStart();
			for (i = 0; i < count; ++i) {
			   if (i % 2)
				printf("          %6d    55555555556666666666777777777788888888889999999999\n",i);
			   else
				printf("          %6d    00000000001111111111222222222233333333334444444444\n",i);
			   do {
				rv = plcTraceDownLoad(buffer);
				if (rv == 0) {
					m = 1;
					for (j = 0; j < trlist_len; ++j) {
						printf("%2d %16s ",j,trlist[j]);
						for (k = 0; k < 50; ++k) {
							if (buffer[k] & m)
								printf("1");
							else
								printf(" ");
						}
						m <<= 1;
						printf("\n");
					}
				}
				else if (rv != PLCERR_NOTREADY) {
					printf("Errore %d\n",rv);
					return 1;
				}
				else {
					usleep(50000);
				}
			    } while (rv);
			}
			plcTraceStop();
		}
		else if (strcmp(p,"show") == 0) {
			plcTraceGetBuf(&len);
			printf("Dimensione del buffer : %2d campioni\n",
			       len);
		}
		else
			return 0;
	} while ( (p = strtok(NULL," \t")) );

	return 1;
}

static int cm_showshvar(char *args)
{
char *p,*q;
struct plcvar_t t;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		printf("%s : ",p);
		if (! dbGetVar(p,&t)) {
			printf("segnale di scambio sconosciuto\n");
		}
		else {
			switch (t.type) {
			case ISAVAR_T_BOOL:
				q = "BOOL";
				break;
			case ISAVAR_T_ANA_I:
				q = "DINT";
				break;
			case ISAVAR_T_TIMER:
				q = "TIME";
				break;
			case ISAVAR_T_ANA_F:
				q = "REAL";
				break;
			default:
				q = "?";
				break;
			}
			printf("type=%s ",q);
			printf("size=%d ",t.size);
			printf("scope=%d ",t.scope);
			printf("attr=%d ",t.attr);
			printf("format=\"%s\",\"%s\"\n",
			       t.format[0],t.format[1]);
		}
	} while ( (p = strtok(NULL," \t")) );

	return 1;
}

static int repeat_count = 1;
static int repeat_on = 0;

static int cm_repeat(char *args)
{
char *p;
unsigned int num;

	if (repeat_on)
		return 1;
	if (! args || !args[0])
		return 0;
	p = args;
	if (sscanf(p,"%u",&num) != 1)
		return 0;
	repeat_count = num;
	repeat_on = 1;
}

static int if_cond = 1;

static int cm_if(char *args)
{
char *p;
struct plcvar_t t;

	if (! args || !args[0])
		return 0;
	p = args;
	if (! dbGetVar(p,&t)) {
		printf("segnale di scambio sconosciuto\n");
		if_cond = 0;
		return -1;
	}
	if (t.type != ISAVAR_T_BOOL) {
		if_cond = 0;
		printf("la variabile deve essere di tipo BOOL\n");
		return -1;
	}
	if_cond = *(char *)t.pval;
	return 1;
}

static int cm_ifn(char *args)
{
int rv;

	rv = cm_if(args);
	if (rv == 1)
		if_cond = !if_cond;
	return rv;
}

static int cm_shell(char *args)
{
char *shell;

	shell = getenv("SHELL");
	if (! shell)
		shell = "/bin/sh";
	system(shell);
	return 1;
}

static int watch_id[] = { -1, -1, -1, -1 };
#define MAX_WATCH (sizeof(watch_id)/sizeof(watch_id[0]))
static watch_descr_t w_param[MAX_WATCH];
struct brk {
	watch_t bp;
	struct brk *next;
};
static struct brk br[MAX_WATCH*10];
static struct brk *bp[MAX_WATCH];
static struct brk *br_free = (struct brk *)0;

static void cm_resetwatches(void)
{
int i;

	i = sizeof(br)/sizeof(br[0]);
	br_free = &br[i - 1];
	while (--i)
		br[i].next = &br[i - 1];
	br[0].next = (struct brk *) 0;
	for (i = 0; i < MAX_WATCH; ++i) {
		watch_id[i] = -1;
		bp[i] = (struct brk *) 0;
	}
}

static void cm_delwatch(int id)
{
struct brk *p,*q;

	for (p = bp[id]; p; p = q) {
		q = p -> next;
		p -> next = br_free;
		br_free = p;
	}
	watch_id[id] = -1;
	bp[id] = (struct brk *) 0;
}

static int cm_wparam(watch_descr_t *param)
{
char *p;
unsigned long v;

	param -> cond = WATCH_C_TRUE;
	param -> act = WATCH_A_INC | WATCH_A_CYC | WATCH_A_LOG;
	param -> apar = 0;
	param -> condact = 0;
	param -> opc = 0;
	param -> opa1 = 0;
	param -> opa2 = 0;

	while ( (p = strtok(NULL," \t")) ) {
		if (p[0] == ';')
			return -1;
		if (strncmp(p,"act=",4) == 0) {
			p = p + 4;
			while (p && *p) {
				if (strncmp(p,"+set",4) == 0)
					param -> act |= WATCH_A_SET;
				else if (strncmp(p,"?set",4) == 0) {
					param -> act |= WATCH_A_SET;
					param -> condact |= WATCH_A_SET; }
				else if (strncmp(p,"-set",4) == 0)
					param -> act &= ~WATCH_A_SET;
				else if (strncmp(p,"+reset",6) == 0)
					param -> act |= WATCH_A_RESET;
				else if (strncmp(p,"?reset",6) == 0) {
					param -> act |= WATCH_A_RESET;
					param -> condact |= WATCH_A_RESET; }
				else if (strncmp(p,"-reset",6) == 0)
					param -> act &= ~WATCH_A_RESET;
				else if (strncmp(p,"+tooggle",8) == 0)
					param -> act |= WATCH_A_TOOGGLE;
				else if (strncmp(p,"?tooggle",8) == 0) {
					param -> act |= WATCH_A_TOOGGLE;
					param -> condact |= WATCH_A_TOOGGLE; }
				else if (strncmp(p,"-tooggle",8) == 0)
					param -> act &= ~WATCH_A_TOOGGLE;
				else if (strncmp(p,"+inc",4) == 0)
					param -> act |= WATCH_A_INC;
				else if (strncmp(p,"?inc",4) == 0) {
					param -> act |= WATCH_A_INC;
					param -> condact |= WATCH_A_INC; }
				else if (strncmp(p,"-inc",4) == 0)
					param -> act &= ~WATCH_A_INC;
				else if (strncmp(p,"+cyc",4) == 0)
					param -> act |= WATCH_A_CYC;
				else if (strncmp(p,"?cyc",4) == 0) {
					param -> act |= WATCH_A_CYC;
					param -> condact |= WATCH_A_CYC; }
				else if (strncmp(p,"-cyc",4) == 0)
					param -> act &= ~WATCH_A_CYC;
				else if (strncmp(p,"+logfl",6) == 0)
					param -> act |= WATCH_A_LOGFL;
				else if (strncmp(p,"?logfl",6) == 0) {
					param -> act |= WATCH_A_LOGFL;
					param -> condact |= WATCH_A_LOGFL; }
				else if (strncmp(p,"-logfl",6) == 0)
					param -> act &= ~WATCH_A_LOGFL;
				else if (strncmp(p,"+log",4) == 0)
					param -> act |= WATCH_A_LOG;
				else if (strncmp(p,"?log",4) == 0) {
					param -> act |= WATCH_A_LOG;
					param -> condact |= WATCH_A_LOG; }
				else if (strncmp(p,"-log",4) == 0)
					param -> act &= ~WATCH_A_LOG;
				else if (strncmp(p,"+brk",4) == 0)
					param -> act |= WATCH_A_BRK;
				else if (strncmp(p,"?brk",4) == 0) {
					param -> act |= WATCH_A_BRK;
					param -> condact |= WATCH_A_BRK; }
				else if (strncmp(p,"-brk",4) == 0)
					param -> act &= ~WATCH_A_BRK;
				else
					return 0;
				p = strchr(p,',');
				if (p)
					++p;
			}
			continue;
		}
		if (strncmp(p,"cond=",5) == 0) {
			p = p + 5;
			if (strcmp(p,"false") == 0)
				param -> cond = WATCH_C_FALSE;
			else if (strcmp(p,"true") == 0)
				param -> cond = WATCH_C_TRUE;
			else if (strcmp(p,"and") == 0)
				param -> cond = WATCH_C_AND;
			else if (strcmp(p,"or") == 0)
				param -> cond = WATCH_C_OR;
			else if (strcmp(p,"nor") == 0)
				param -> cond = WATCH_C_NOR;
			else if (strcmp(p,"tgt") == 0)
				param -> cond = WATCH_C_TGT;
			else if (strcmp(p,"tle") == 0)
				param -> cond = WATCH_C_TLE;
			else if (strcmp(p,"teq") == 0)
				param -> cond = WATCH_C_TEQ;
			else if (strcmp(p,"cgt") == 0)
				param -> cond = WATCH_C_CGT;
			else if (strcmp(p,"cle") == 0)
				param -> cond = WATCH_C_CLE;
			else if (strcmp(p,"rz") == 0)
				param -> cond = WATCH_C_RZ;
			else if (strcmp(p,"nrz") == 0)
				param -> cond = WATCH_C_NRZ;
			else if (strcmp(p,"mz") == 0)
				param -> cond = WATCH_C_MZ;
			else if (strcmp(p,"nmz") == 0)
				param -> cond = WATCH_C_NMZ;
			else
				return 0;
			continue;
		}
		if (strncmp(p,"apar=",5) == 0) {
			p = p + 5;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			param -> apar = v;
		}
		else if (strncmp(p,"opc=",4) == 0) {
			p = p + 4;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			param -> opc = v;
		}
		else if (strncmp(p,"opa1=",5) == 0) {
			p = p + 5;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			param -> opa1 = v;
		}
		else if (strncmp(p,"opa2=",5) == 0) {
			p = p + 5;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			param -> opa2 = v;
		}
		else
			return 0;
	}
	return 1;
}

static int cm_addbrk(int id)
{
char *p;
watch_t w;
unsigned long v;
struct brk *b,*q,*r;
int rv;

	w.addr = (char *) 0;
	w.operand.r1 = 0;
	w.operand.r2 = 0;
	w.operand.scale = 0;
	w.operand.size = 0;
	w.operand.off = 0;
	w.operand.ar = 0;

	while ( (p = strtok(NULL," \t")) ) {
		if (p[0] == ';')
			return -1;
		if (strncmp(p,"addr=",5) == 0) {
			p = p + 5;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.addr = (unsigned char *) v;
		}
		else if (strncmp(p,"r1=",3) == 0) {
			p = p + 3;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.r1 = v;
		}
		else if (strncmp(p,"r2=",3) == 0) {
			p = p + 3;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.r2 = v;
		}
		else if (strncmp(p,"scale=",6) == 0) {
			p = p + 6;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.scale = v;
		}
		else if (strncmp(p,"size=",5) == 0) {
			p = p + 5;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.size = v;
		}
		else if (strncmp(p,"off=",4) == 0) {
			p = p + 4;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.off = v;
		}
		else if (strncmp(p,"ar=",3) == 0) {
			p = p + 3;
			if (sscanf(p,"0x%lx",&v) != 1
			 && sscanf(p,"%lu",&v) != 1)
				return 0;
			w.operand.ar = v;
		}
		else
			return 0;
	}
	if (watch_id[id] == -1) {
		printf("Lista di watch %d indefinita !\n",
		       id);
		return 1;
	}
	b = br_free;
	if (! b)
		printf("Lista di break esaurita !\n");
	else {
		br_free = br_free -> next;
		b -> bp = w;
		for (q = bp[id], r = (struct brk *) 0; q; r = q, q = q -> next)
			;
		if (! r)
			bp[id] = b;
		else
			r -> next = b;
		b -> next = (struct brk *) 0;
		rv = plcAddBreak(watch_id[id],&w);
		if (rv)
			printf("Errore %d\n",rv);
	}
	return 1;
}

static void cm_showbrk(void)
{
brk_event_list *el;
brk_event *ev;
char *p;
unsigned int n;
int id,i,size = 0;
static char *regs[] = { "","eax","ebx","ecx","edx","edi","esi","ebp","esp" };
struct brk *brp;

	for (;;) {
		if ( (el = plcLastDebugEventList()) ) {
			while ( (ev = plcGetDebugEvent()) ) {
				for (id = 0; id < MAX_WATCH; ++id)
					if (watch_id[id] == ev -> be_id)
						break;
				if (id >= MAX_WATCH) {
					printf("Evento sconosciuto : %u\n",
					       (unsigned int)(ev -> be_id));
					break;
				}
				printf("cycle=0x%08lx break=%2x\n",el -> bl_cycle,id);
				for (p = ev -> be_data,
				       n = ev -> be_size - 3,
				       brp = bp[id];
				     n && brp;
				     n -= size, brp = brp -> next) {
					size = 0;
					printf(" addr=0x%08lx",brp -> bp.addr);
					if (brp -> bp.operand.scale == 0) {
						if (brp -> bp.operand.r1) {
							printf(" %s=0x%08lx",
							   regs[brp -> bp.operand.r2],
							   *(unsigned long *)p);
							size += 4;
							p += 4;
						}
						if (brp -> bp.operand.r2) {
							printf(" %s=%08lx",
							   regs[brp -> bp.operand.r2],
							   *(unsigned long *)p);
							size += 4;
							p += 4;
						}
					}
					else {
						printf(" ");
						if (brp -> bp.operand.off)
						   printf("0x%08lx",brp -> bp.operand.off);
						if (brp -> bp.operand.r1
						 || brp -> bp.operand.r2) {
							printf("(%s,%s,%d)",
							  regs[brp -> bp.operand.r1],
							  regs[brp -> bp.operand.r2],
							  brp -> bp.operand.scale);
						}
						printf("=");
						switch (brp -> bp.operand.size)
						{
						case 1:
							printf("0x%02x",
							 *(unsigned char*)p);
							break;
						case 2:
							printf("0x%04x",
							 *(unsigned short*)p);
							break;
						case 4:
							printf("0x%08lx",
							 *(unsigned long*)p);
							break;
						default:
							printf("0x%08lx%08lx",
							 *(unsigned long*)(p+4),
							 *(unsigned long*)p);
							break;
						}
						size += brp -> bp.operand.size;
						p += brp -> bp.operand.size;
					}
					if (brp -> bp.operand.ar) {
						printf(" %s=%08lx",
						   regs[brp -> bp.operand.ar],
						   *(unsigned long *)p);
						size += 4;
						p += 4;
					}
					printf("\n");
				}
			}
		}
	}
}

static int cm_watch(char *args)
{
char *p,*q;
int id,wid,rv;

	if (! args || !args[0])
		return 0;
	p = args;
	do {
		if (p[0] == ';')
			return -1;
		if (strcmp(p,"reset") == 0) {
			plcClearAllWatches();
			cm_resetwatches();
			continue;
		}
		if (strcmp(p,"show") == 0) {
			cm_showbrk();
			continue;
		}
		if (! (q = strtok(NULL," \t"))
		   || sscanf(q,"%d",&id) != 1 || id < 0 || id >= MAX_WATCH)
			return 0;
		if (strcmp(p,"create") == 0) {
			if (! cm_wparam(&w_param[id]))
				return 0;
			if (watch_id[id] != -1) {
				printf("Lista di watch %d gia` definita !\n",
				       id);
				continue;
			}
			if ( (rv = plcCreateWatchList(&w_param[id],&wid)) )
				printf("Errore %d\n",rv);
			else {
				watch_id[id] = wid;
				bp[id] = (struct brk *) 0;
				printf("Watch %d -> %d\n",id,wid);
			}
		}
		else if (strcmp(p,"delete") == 0) {
			if (watch_id[id] == -1) {
				printf("Lista di watch %d gia` cancellata !\n",
				       id);
				continue;
			}
			if ( (rv = plcDeleteWatchList(watch_id[id])) )
				printf("Errore %d\n",rv);
			else
				cm_delwatch(id);
		}
		else if (strcmp(p,"enable") == 0) {
			if (watch_id[id] == -1) {
				printf("Lista di watch %d indefinita !\n",
				       id);
				continue;
			}
			if ( (rv = plcEnableWatchList(watch_id[id])) )
				printf("Errore %d\n",rv);
		}
		else if (strcmp(p,"disable") == 0) {
			if (watch_id[id] == -1) {
				printf("Lista di watch %d indefinita !\n",
				       id);
				continue;
			}
			if ( (rv = plcDisableWatchList(watch_id[id])) )
				printf("Errore %d\n",rv);
		}
		else if (strcmp(p,"add") == 0) {
			if (! cm_addbrk(id))
				return 0;
		}
		else
			return 0;
	} while ( (p = strtok(NULL," \t")) );
	return 1;
}

struct command_t {
	char *str;
	int (*f)(char *args);
	char *hlp;
} command_table[] = {
{ "?", cm_help, NULL },
{ "help", cm_help, "[cmd] : riporta sintassi ed uso del comando \"cmd\"" },
{ "dump", cm_dump, "[0x]<addr> [0x]<size> : mostra il contenuto di\nun'area del Plc all'indirizzo \"addr\" e della dimensione \"dim\"" },
{ "exit", cm_exit, NULL },
{ "quit", cm_exit, ": uscita dal programma" },
{ "halt", cm_halt, ": mette in HALT l'esecutore" },
{ "go", cm_go, ": mette in GO l'esecutore" },
{ "if", cm_if, "<var> <cmd> : esegue il comando se il segnale di scambio \"var\" vale TRUE" },
{ "ifn", cm_ifn, "<var> <cmd> : esegue il comando se il segnale di scambio \"var\" vale FALSE" },
{ "load", cm_load, "<file> : carica dinamicamente un programma" },
{ "mod", cm_module, "<name> : riporta lo stato del modulo specificato" },
{ "print", cm_shvar, "<var> : stampa il valore del segnale di scambio \"var\"" },
{ "repeat", cm_repeat, "<num> <cmd> : ripete il comando \"cmd\" \"num\" volte,\nad intervalli di mezzo secondo" },
{ "set", cm_setshvar, "<var> <val> : assegna il valore \"val\" al un segnale di scambio \"var\"" },
{ "show", cm_showshvar, "<var> : stampa le caratteristiche del segnale di scambio \"var\"" },
{ "sh", cm_shell, NULL },
{ "shell", cm_shell, ": lancia una shell interattiva" },
{ "st", cm_status, NULL },
{ "status", cm_status, ": stato corrente del Plc" },
{ "time", cm_time, "<opzione> [opzione...] : gestione della statistica.\n"
"    \"opzione\" puo` avere le forme seguenti :\n"
"        clear : riazzera la statistica del Plc.\n"
"        on : abilita la statistica.\n"
"        off : disabilita la statistica.\n"
"        show : visualizza la statistica." },
{ "trace", cm_trace,
"{clear,add <var>,list,start <n>,show} : azzera il trace,\n"
"aggiunge una variabile, visualizza la lista, attende e stampa \"n\" buffer,\n"
"mostra lo stato" },
{ "unload", cm_unload, ": elimina l'ultimo modulo caricato" },
{ "watch", cm_watch, "<opzione> [param...] : manipolazione dei breakpoint.\n"
"    \"opzione\" puo` avere le forme seguenti :\n"
"        reset : cancella tutti i breakpoint impostati\n"
"        create <id> [parametri] : crea una nuova lista di breakpoint\n"
"          \"id\" e` un numero compreso tra 0 e 3.\n"
"          \"parametri\" puo` essere una dei seguenti :\n"
"         act=<azione>[,<azione>...]\n"
"          dove \"azione\" = {+|-|?}{set|reset|tooggle|inc|cyc|log|logfl|brk}\n"
"         cond={false|true|and|or|nor|tgt|tle|teq|cgt|cle|ceq|rz|nrz|mz|nmz}\n"
"         {apar|opc|opa1|opa2}=<valore>\n"
"        add <id> addr=<indirizzo> [operandi] : aggiunge un breakpoint\n"
"          alla lista. \"operandi\" e` una lista dei seguenti :\n"
"         {r1|r2|ar}=<indice registro>\n"
"         {off|scale|size}=<valore>\n"
"        {delete,enable,disable} <indice> : cancella, abilita o disabilita\n"
"         una lista di watch\n"
"        show : attende eventi di break e li visualizza"
},
};

static void cm_ver(void)
{
	printf("\ntester per qplc/isaker versione %s.\n",VERSIONE);
}

static int cm_help(char *args)
{
int i,prima;
char *p;

	if (! args || !args[0]) {
		cm_ver();
		printf("\n Comandi riconosciuti :\n");
		for (i = 0; i < sizeof(command_table)/sizeof(command_table[0]); ++i) {
			if (i % 7 == 0)
				printf("\n");
			printf("%10s",command_table[i].str);
		}
		printf("\n");
	}
	else {
		p = args;
		do {
			if (p[0] == ';')
				break;
			prima = 0;
			for (i = 0; i < sizeof(command_table)/sizeof(command_table[0]); ++i) {
				if (strcmp(p,command_table[i].str) == 0
				    || prima) {
					if (command_table[i].hlp) {
						printf("%s%s %s\n",
						        prima ? "" : "  ",
							command_table[i].str,
							command_table[i].hlp);
						prima = 0;
					}
					else {
						printf("  %s|",command_table[i].str);
						prima = 1;
					}
				}
			}
		} while ( (p = strtok(NULL," \t")) );
	}
	return 1;
}

static jmp_buf entry_point;
static int interr = 0;

static void cm_intr(int s)
{
	interr = 1;
	longjmp(entry_point,1);
}

static void process_command(char *cmd)
{
int i,rv;
char *p;
char *line;

	repeat_count = 1;
	repeat_on = 0;
	interr = 0;
	if_cond = 1;

	if (! cmd)
		return;
	line = strdup(cmd);
	if (! line)
		return;
	for (;;) {
	    if (! (p = strtok(line," \t")) )
		break;
	    if_cond = 1;
	    while (if_cond) {
		for (i = sizeof(command_table)/sizeof(command_table[0]);
		     --i >= 0; ) {
			if (strcmp(p,command_table[i].str) == 0) {
				rv = (*command_table[i].f)(strtok(NULL," \t"));
				break;
			}
		}
		if (strcmp(p,";") == 0) {
			if ( (p = strtok(NULL," \t")) )
				continue;
			else
				break;
		}
		if (i < 0) {
			printf("Comando sconosciuto. Prova \"help\" o \"help <comando>\"\n");
			repeat_count = 1;
			break;
		}
		else if (! rv) {
			printf("Errore di sintassi. Prova \"help\" o \"help <comando>\"\n");
			repeat_count = 1;
			break;
		}
		if ( ! (p = strtok(NULL," \t")) )
			break;
	    }
	    if (! --repeat_count) {
		break;
	    }
	    usleep(500000L);
	    strcpy(line,cmd);
	}
	free(line);
}

int main(int argc, char **argv)
{
char *p;
static char help[] = "help";
static char status[] = "status";

	plcOpen();

	if (AttachNvram(NVRAM_LBL_PLC480,&plc_nvram,NULL,0) >= 0) {
		dbRegisterNvram(plc_nvram.addr);
	}

	plcTraceStop();
	plcTraceClear();
	plcGetExeName(exe_path);
	cm_ver();
	cm_resetwatches();
	printf("\nScrivi \"help\" o \"?\" per avere la lista dei comandi.\n\n");
	process_command(status);

	setjmp(entry_point);

	signal(SIGINT,cm_intr);
	signal(SIGQUIT,cm_intr);

	for (;;) {
		if (! (p = rl_gets()) )
			continue;
		process_command(p);
	}

	return 0;
}


