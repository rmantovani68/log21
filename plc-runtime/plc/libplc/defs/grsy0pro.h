/****************************************************************************
* File        : grsy0pro.h
* Description : All prototypes
* Creation    : LIS 20/02/93
*****************************************************************************
* Modif       : 04/04/94 DEF_ILDBG
*****************************************************************************/

/***************************************************************************
* Description : System procedures
***************************************************************************/

extern void sys_arg(int argc, char *argv[], char *com, 
                     byte *s1,byte *s2, byte *s3, byte *s4);

extern void sys_initerr(byte detect);
extern void sys_readerr(word *err1, word *err2);
#ifndef sys_err
extern void sys_err(int err1, long err2);
#endif
extern byte sys_testerr(void);

extern long sys_hex_term(char *mess);
extern long sys_dec_term(char *mess);
extern float sys_real_term(char *mess);
extern void sys_bin_term(long val, short nb_bit, char *mess);
extern short sys_menu_term(char *label);

extern short sys_strlen(char *txt);
extern void sys_strcpy(char *dst, char *src);
extern void sys_strcat(char *dst, char *src);

extern void lib_rsv_init(byte *add);
extern byte *lib_rsv_read(void);
extern byte *lib_rsv(long size);
extern void lib_reset(byte *add, long size);

extern void sys_reset_bit(byte *add, byte nbit);
extern byte sys_test_bit (byte *add, byte nbit);
extern byte sys_tas_bit  (byte *add, byte nbit);
extern void sys_set_bit  (byte *add, byte nbit);
extern void sys_wreset_bit(byte *add, word nbit);
extern byte sys_wtest_bit (byte *add, word nbit);
extern byte sys_wtas_bit  (byte *add, word nbit);
extern void sys_wset_bit  (byte *add, word nbit);
extern byte sys_lock_sem  (byte *add);
extern void sys_unlock_sem  (byte *add);

extern void sys_readtim(unsigned long *tim);
extern void sys_waittim(long tim);

extern byte sys_del_data(char *name);
extern byte sys_read_data(char *name, long add, byte size, byte *buf);
extern byte sys_write_data(char *name, long add, byte size, byte *buf);

extern byte sys_creat_spc(byte slave, byte num, char **add, long siz);
extern byte sys_size_spc(byte slave, byte num, char **add, long *siz);
extern byte sys_link_spc(byte slave, byte num, char **add);
extern void sys_del_spc(byte slave, byte num);
extern byte sys_load_spc(byte slave, byte num, char **add);
extern byte sys_save_spc(byte slave, byte num);

extern void sys_checkmod(char *add); 
extern byte sys_creemod(char *nam, char **add, long siz); 
extern byte sys_sizemod(char *nam, char **add, long *siz); 
extern void sys_delmod(char *nam); 

extern void sys_flushcom(void);
extern void sys_pausecom(void);
extern byte sys_closecom(void);
extern byte sys_testcom(void);
extern byte sys_getcom(byte *buffer);
extern byte sys_readbcom(byte *buffer, long timeout);
extern byte sys_waitbcom(byte *buffer);
extern byte sys_readcom(byte *buffer, byte nboct);
extern byte sys_writecom(byte *buffer, byte nboct);
extern byte sys_opencom(char *desc);
extern byte sys_configcom(void);

extern void sys_init_crc(void);
extern void sys_set_crc(byte *buf, byte size);
extern byte sys_check_crc(byte *buf, byte size);

extern long *sys_load_exe(byte num, long *base);
extern byte sys_load_data(char *mod, char **add);
extern byte sys_save_data(char *mod, char *fic, long max);
extern void sys_delfic(char *name);

extern byte sys_start_task(char *name, char *arg1, long *pid); 
extern byte sys_stop_task(long pid); 
extern byte sys_sig_task(long pid, long sig); 
extern void sys_intercept(void);

/***************************************************************************
* Description : Ios procedures
***************************************************************************/

extern byte  ios_attr(word num, byte typ);
extern short ios_init(byte slave);
extern long  ios_boo_operate(word va, long code, long val);
extern long  ios_ana_operate(word va, long code, long val);
extern long  ios_msg_operate(word va, long code, long val);
extern byte  ios_open(void);
extern void  ios_input(void);
extern void  ios_output(void);
extern void  ios_close(void);
extern void  ios_tab_init(void);
extern void  ios_error(byte err, void *brd);

extern byte  cnv_init(byte *base, word nb);
extern void  cnv_tab_inp(void *arg);
extern void  cnv_tab_out(void *arg);
extern void  cnv_call(void *arg);

extern void ios_boo_ref(word num);
extern void ios_ana_ref(word num);
extern void ios_msg_ref(word num);

extern void ios_outboo(word num);
extern void ios_outana(word num);
extern void ios_outmsg(word num);

extern void ios_inp_brd(void);
extern void ios_out_brd(void);
extern void ios_open_brd(void);
extern void ios_close_brd(void);

extern void ios_conv_input(void *);
extern void ios_conv_output(void *);

/***************************************************************************
* Description : Tester procedures
***************************************************************************/

byte lv2_ecr_trame (byte *buf);
byte lv2_ecr_modbus (byte *buf, byte size);
byte lv2_lec_trame (byte **buf);
byte lv2_test_trame (void);
byte lv2_test(void);   
byte lv2_close(void);   
byte lv2_init(char *desc, byte esc1, byte esc2);

byte lv3_ecr_trame (byte *buf, byte nboct, byte typreq);
byte lv3_lec_trame (byte *typreq, byte **buf);
byte lv3_ready(void);   
byte lv3_init(char *desc, byte numesc1, byte numesc2);

byte lv4_ecr_trame (short nboct, byte typ);
byte lv4_lec_trame (byte *typ);
byte lv4_init (char *desc, byte numesc1, byte numesc2);
short lv4_transfert(byte *add, long taille);

byte lv5_session(byte *ques, byte *ans);
byte lv5_load(byte *ques, byte *ans);

short com_question(byte *ques, byte *ans, long tout);
byte *com_link(byte slave);

void com_init(char *com, byte slave);
byte com_read(byte **ques, byte **ans);
void com_write(short size);
void com_close(void);

short mod_charge(void);
short mod_maj(void);

extern short she_start(char *name);

/***************************************************************************
* Description : Modbus procedures
***************************************************************************/

extern void mdb_rbit(byte *buf);
extern void mdb_wbit(byte *buf);
extern void mdb_wword(byte *buf);
extern void mdb_wnword(byte *buf);
extern void mdb_rword(byte *buf);
extern byte mdb_rdata(byte *ques, byte *ans);
extern byte mdb_wdata(byte *ques, byte *ans);
extern short mdb_answer(byte typ, byte *buf, short size);

/* eof */
