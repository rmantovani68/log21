
#include <stdio.h>

#include "ilcrtm.h"

void dump_msd(il_msd_t *msd)
{
char *op;
int sz;

	for ( ;
	     msd && msd -> flags != ILC_END_LIST;
	     msd = (il_msd_t *)(((char *)&msd -> data) + msd -> data_size)) {

	switch (msd -> flags & ILC_MSD_SIZE_M) {
	case ILC_MSD_SIZE_1:	sz = 1; break;
	case ILC_MSD_SIZE_8:	sz = 8; break;
	case ILC_MSD_SIZE_16:	sz = 16; break;
	case ILC_MSD_SIZE_32:	sz = 32; break;
	case ILC_MSD_SIZE_64:	sz = 64; break;
	default: sz = 0; break;
	}
	op = ":=";
	switch (msd -> flags & ILC_MSD_BIND_M) {
	case ILC_MSD_BIND_OUT:
		op = "=>";
	case ILC_MSD_BIND_IN:
		printf("    0x%08lx %s %s, %d bit\n",
		       msd -> target, op, msd -> data.io,sz);
		break;
	case ILC_MSD_BIND_MEM:
		printf("    %s [%d, %d] at ",
		       msd -> data.shv.name, msd -> data.shv.dim1,
		       msd -> data.shv.dim2);
		if (msd -> flags & ILC_MSD_MEMVAR_FIXED) {
			printf("key %ld",msd -> target);
		}
		else {
			printf("var 0x%08lx",msd -> target);
		}
		printf(", %d %c\n",sz,
		       (msd -> flags & ILC_MSD_MEMVAR_FLOAT) ? 'F' : 'I');
		break;
	default:
		printf("    0x%08lx forced to ", msd -> target);
		switch (sz) {
		case 1: printf("%s\n",msd -> data.cv ? "TRUE":"FALSE"); break;
		case 8: printf("0x%02x\n",msd -> data.cv); break;
		case 16: printf("0x%04x\n",msd -> data.hv); break;
		case 32: printf("0x%08lx\n",msd -> data.lv); break;
		case 64: printf("0x%08lx,",msd -> data.lv);
		         printf("0x%08lx\n",*(&msd -> data.lv + 1)); break;
		}
	}

	}
}

void main(void)
{
il_module_t *mod;
il_config_chain_t *chain;
il_config_t *conf,**pconf;
il_resource_t *res,**pres;
il_program_t *prog,**pprog;
il_msd_t *msd;

	mod = &il_module;

	printf("Module \"%s\"\n",mod -> magic);
	for (chain = mod -> chain; chain; chain = chain -> next) {
		if (chain -> next)
			printf(" Global MSD : \n");
		dump_msd(chain -> msd);
		for (pconf = chain -> conf; *pconf; ++pconf) {
			conf = *pconf;
			printf(" Configuration \"%s\" :\n",conf -> name);
			for (pres = conf -> resource; *pres; ++pres) {
				res = *pres;
				printf("  Resource \"%s\" on \"%s\" :\n",
				       res -> name,res -> on);
				for (pprog = res -> prog; *pprog; ++pprog) {
					prog = *pprog;
					printf("   Program 0x%08lx(0x%08lx) :\n",prog -> program,prog -> statics);
					dump_msd(prog -> msd);
				}
			}
		}
	}
}

