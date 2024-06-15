 
#define MAX_MSG 8 
#define MAX_PTA 8 
#define MAX_PRIORITE 19 
#define MAX_ACTIVITE 5 
#define MAX_RECEPT 5 
#define MAX_SIM_ACT 6 
#define MAX_SIM_REP 4  
#define MAX_BKP 16 

#define BKP_BAD 0  
#define BKP_DEB 1 
#define BKP_FIN 2 
#define BKP_FRA 3 

/*
Rappezzamento
#define S_PUSH(val) *(long *)STACK++= (long)(val)
#define S_PUSHF(val) *(float *)STACK++= (float)(val)
*/
#define S_PUSH(val) (					\
{							\
	long __tmp_val = (long)(val);			\
	*(long *)STACK++ = __tmp_val;			\
})
#define S_PUSHF(val) (					\
{							\
	float __tmp_val = (float)(val);			\
	*(float *)STACK++ = __tmp_val;			\
})

#define S_POP() (*(long *)(--STACK))
#define S_POPF() (*(float *)(--STACK))

#define S_SET(val) *(STACK-1)= (long)(val)
#define S_WRITE(val) (*(STACK-1))= (long)(val) 
#define S_NOT() (*(STACK-1))= !(*(STACK-1))

#define STS(x) (*(KER->st_gra + x)) 

#define BOO_SET(va) (BF_BOO[va]= 1)
#define BOO_RESET(va) (BF_BOO[va]= 0)
#define BOO_GET(va) (BF_BOO[va])
#define ANA_ADD(va) (BF_ANA+(va))

