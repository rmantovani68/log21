 
#define ERR_EXE 1 
#define ERR_SLV 2 
#define ERR_REQ 3 
#define ERR_OUT 4 

#ifndef MAX_TRAME
#define MAX_TRAME 255 
#endif
#define T_MODBUS 20  
#define T_R_NBIT 1 
#define T_R_NWORD 3
#define T_W_BIT 5
#define T_W_WORD 6
#define T_W_NBIT 15
#define T_W_NWORD 16
#define T_W_DATA 17 
#define T_R_DATA 18 
#define MDB_NAME 0xf000 
#define MDB_PAGE 0xf002 
#define MDB_DEL 0xf004 

#define MODBUS_ERR (byte)0x80 
#define MODBUS_NUM (byte)01  
#define MODBUS_ADD (byte)02  
#define MODBUS_NRDY (byte)04  

extern short mst_init(char *desc);
extern short mst_close(void);
extern short mst_send_readb (byte slv, word address, byte nbbits);
extern short mst_get_readb (byte slv, byte *nbw, byte *frame, long timeout);
extern short mst_send_readw (byte slv, word address, byte nbw);
extern short mst_get_readw (byte slv, byte *nbw, byte *frame, long timeout);
extern short mst_send_readblock(byte slv, word address, byte nbw);
extern short mst_get_readblock(byte slv, byte *nbw, byte *frame, long timeout);
extern short mst_send_write (byte slv, word address, word valeur);
extern short mst_get_write (byte slv, word *address, word *valeur, long timeout);
extern short mst_send_writeb(byte slv, word address, word valeur);
extern short mst_get_writeb (byte slv, word *address, word *valeur, long timeout);
extern short mst_send_writew(byte slv, word address, byte *frame, byte nbw);
extern short mst_get_writew (byte slv, word *address, byte *nbw, long timeout);
extern short mst_send_writeblock(byte slv, word address, byte *frame, byte nbw);
extern short mst_get_writeblock(byte slv, word *address, byte *nbw, long timeout);
extern void mst_initerr(void);
extern short mst_err (byte *err);
extern byte slv_init (char *desc, byte slv);
extern byte slv_close(void);
extern void slv_snd (byte fonction, byte *frame, byte nbb);
extern void slv_rcv (byte *fonction, byte *frame, byte *nbb);

