 
#pragma pack(1)
typedef struct 
{
long offset_fils; 
word trs_init; 
word stp_init; 
word nb_trs; 
word nb_stp; 
long base;  
} str_dfgra;
typedef struct 
{
word seq_debut;  
word seq_comp; 
word seq_fin;  
word trans_avale;  
word grafcet; 
byte priorite; 
byte reserve; 
} str_dfstp;
typedef struct 
{
word seq_recep; 
word step_amont;  
word step_aval;  
word grafcet;  
} str_dftrs;
typedef struct
{
word libno; 
char name[10];  
} str_dfusf;
typedef struct
{
word libno; 
word nbinst; 
char name[10];  
} str_dffbl;
typedef struct
{
float x;
float y;
} str_point;
#define MAX_CNV_T 128
#define MAX_CNV_P 255
#define MAX_PTS 32
typedef struct 
{
unsigned short num;
unsigned short nb_points;
str_point liste[ MAX_PTS ];
} str_table;
typedef struct 
{
 
char head_prod[8]; 
char head_vers[6];  
char head_target[6]; 
char head_appli[16]; 
long head_crc; 
 
word nb_graf_ladd; 
word nb_ladd; 
word nb_ladd_beg;
word nb_ladd_end;
word nb_graf_main; 
short prio_min;  
short prio_max;  
word nb_step; 
word nb_tran; 
word nb_boo_total; 
word nb_boo_intern; 
word nb_boo_idir; 
word nb_boo_odir; 
word reserve1;
word reserve2; 
word nb_ana_total; 
word nb_ana_intern;
word nb_ana_idir;
word nb_ana_odir;
word reserve3; 
word reserve4; 
word nb_tmr_total; 
word nb_tmr_intern;
word reserve5;
word appl_version; 

long appl_date_time; 

word nb_msg_total; 
word nb_msg_intern;
word nb_msg_idir;
word nb_msg_odir;
word reserve9;
word reserve10;
word nb_brd; 
word nb_lst; 
word error_chk; 
word nb_usp; 
word nb_tb_cnv; 
word nb_fc_cnv;
word nb_fbl; 
word nb_realcnv; 
word tsk_debug;
word start_status; 
word start_mode;  
word tcycle;  

long graf_base; 
long def_grafcet;
long def_step;
long def_tran;
long lng_msg; 
long io_boo; 
long io_ana;
long io_msg;
long io_brd; 
long tab_cnv; 
long reserve12a; 
long reserve12b; 
long def_modbus; 
long def_usf; 
long def_cnv; 
long init_boo; 
long init_ana; 
long init_tmr; 
long init_msg; 
long def_fbl; 
long reserve15; 
long reserve16;
long reserve17;
long reserve18;
long reserve19;
long reserve20;
long reserve21;
} str_appli;
#ifdef INTEL
#define TARGET_VERS "80x86"
#endif
#ifdef MOTOROLA
#define TARGET_VERS "680x0"
#endif
#ifdef DEF_COMPACT
#define APPLI ((str_appli *)BASE_RAM + sizeof(str_system) + MAX_KER)
#else
extern str_appli *APPLI;
#endif

#pragma pack()

