/* QUESTO FILE E` STATO GENERATO IN MODO AUTOMATICO A PARTIRE DA scanner_c.h */
/* NON APPORTARE MODIFICHE QUI, MA SOLO IN scanner.c */
static int scIsBlank(char c);
static int scIsSymF(char c);
static int scIsSymM(char c);
static int scPreprocess(void);
static LPCHAR scBlanks(void);
static int scBGetCh(void);
static int scBNextCh(char c);
static int scBMatchCh(int c);
static int scBIdent(LPCHAR s);
static int scRawIdent(LPCHAR s);
static int scBUInteger(LPCHAR s);
static int scBInteger(LPCHAR s);
