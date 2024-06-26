 

extern void act_comp(word stp);
extern void act_deb (word stp);
extern void act_fin (word stp);
extern void act_exec(short priorite);
extern void act_init(word max);
extern void bkp_mic(word *mic);
extern void bkp_init(void);
extern long ker_size(void);
extern void ker_create(void);
extern void ker_init(void);
extern long dec_mic(word *mic);
extern long dec_sub(word num);
extern void dec_init(void);
extern void evo_cycle (void);
extern byte fbl_init(byte *base, word nb, byte slave);
extern long fbl_allocate(void);
extern word *fbl_active(word *mic);
extern void fbl_readpa(word *mic);
extern byte gra_kill(word num);
extern byte gra_start(word num);
extern byte gra_rstart(word num);
extern byte gra_freeze(word num);
extern byte gra_rstart(word num);
extern long gra_stat(word num);
extern void ini_var(void);
extern void ini_gra(void);
extern long ild_init(void);
extern word *ild_dec(word *mic, long *val);
extern short ild_info(short *line, long *accu);
extern byte ild_install(word va, word line);
extern void ild_remove(word va);
extern void ild_accu(long val);
extern void ild_debug(void);
extern void log_init(void);
extern void log_begin(void);
extern void log_end(void);
extern void mdf_init(byte slave);
extern byte mdf_load(void);
extern byte mdf_update(void);
extern byte mdf_version(byte *b1, byte *b2, byte *b3);
extern char *msg_add(word va);
extern void msg_write(word num, byte len, byte *str); 
extern long msg_cat(long m1, long m2);
extern short msg_compare(long m1, long m2);
extern word *msg_const(word *mic, long *val);
extern void msg_free(char *msg);
extern char *msg_get(void);
extern void msg_init(void);
extern long msg_mta(long add);
extern long msg_atm(long val);
extern long msg_btm(long val);
extern void msg_pop(long add, word va);
extern long msg_push(word va);
extern void pop_intana(word nb_var);
extern void pop_tmr (word nb_var);
extern void pop_intinc(word nb_var, long val);
extern void pop_outinc(word nb_var, long val);
extern long sav_init(long add, byte typ);
extern void sav_cycle(void);
extern byte sav_save(byte typ);
extern byte sav_restore(byte typ);
extern void stp_mkinit (void);
extern byte stp_marque (word va);
extern byte stp_active (word va);
extern void stp_desactive(word va);
extern byte stp_testact (word va);
extern long stp_readtim (word va);
extern void sts_init(void);
extern void sts_cycle(void);
extern void sts_write(word va, byte status);
extern long sys_call(long code, long arg);
extern void tcy_reset(long tcy);
extern void tcy_begin(void);
extern void tcy_end(void);
extern void tcy_init(long autorise);
extern long tmr_read(word va);
extern void tmr_write(word va, long val);
extern byte tmr_testact(word va);
extern byte tmr_active(word va);
extern byte tmr_deactive(word va);
extern void trs_init(void);
extern byte trs_marque (word va);
extern byte trs_tcc(byte *ques, byte *ans);
extern void trs_tuc(byte *ques);
extern byte usf_init(byte *base, word nb, byte slave);
extern word *usf_mic(word *mic);
extern void usp_call(long *param);

#define STATIC (byte)0 
#define RANY (byte)1 
#define USER (byte)2 
extern short srv_dsp(byte *ques, byte *ans);
extern void srv_wboo (byte *lec);
extern void srv_wilr (byte *lec);
extern void srv_wana (byte *lec);
extern void srv_wtmr (byte *lec);
extern void srv_wmsg (byte *lec);
extern void srv_wlist (byte *lec);
extern short srv_usadd (byte *lec, byte *ecr);
extern short srv_wany (byte *lec, byte *ecr);
extern short srv_usread(byte *lec, byte *ecr);
extern short srv_gstart(byte *lec);
extern short srv_gfreeze(byte *lec);
extern short srv_grstart(byte *lec);
extern short srv_gstart(byte *lec);
extern short srv_gkill(byte *lec);
extern short srv_tstart(byte *lec);
extern short srv_tstop (byte *lec);
extern short srv_slr (byte *lec, byte *ecr);
extern short srv_gstat(byte *lec, byte *ecr);
extern short srv_lst (byte *lec, byte *ecr);
extern short srv_tcy (byte *lec, byte *ecr);
extern void srv_io_unlock(byte *ques);
extern void srv_io_lock(byte *ques);
extern short srv_io_lslock(byte *ans);
extern short srv_bkp_set(byte *ques);
extern short srv_bkp_deb(byte *ques);
extern short srv_bkp_fin(byte *ques);
extern short srv_bkp_trs(byte *ques);
extern short srv_bkp_del(byte *ques);
extern short srv_bkp_act(byte *ques);
extern short srv_bkp_inst(byte *ques);
extern void *bkp_getva(word va);
extern void usr_init(void);
extern word usr_getva(word user, byte *typ, byte *attr);
extern word usr_getusad(word va);

