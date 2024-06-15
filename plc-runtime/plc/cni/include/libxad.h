/* LIBXAD
 * MODULE = libxad.h common definitions for the Xwindows Systech Custom Library
 * 		that must be linked to application developped with XAD
 *
 * SysTech srl Torino
 *
 * 20/10/1993 V 1.0 BSD386 & MOTIF v1.2
*/

/* Operation Mode Definitions */
#define XsQUIET		0
#define XsDEBUG_WID	1		/* Log the Widget Creations */
#define XsDEBUG_RES	(1<<1)		/* Log the Resources Settings */
#define XsDEBUG_CB	(1<<2)		/* Log the Callbacks Installations */
#define XsHALT_ON_ERR	(1<<3)		/* On Errors call exit whitout returning */

#define XsError         -1
#define XsMANAGE        0
#define XsNOMANAGE      1
#define XsNOSENSITIVE   (1 << 1)	/* Obsolete not to be used */
#define XsNOMAP         (1 << 2)

extern int XsErrno;		/* Global variable used to store the returns
				 * error codes */
extern char *homePath;

#define XsENODIR	1000	/* Invalid Directory */
#define XsENOFILE	1001	/* No such file */
#define XsENOWID	1002	/* Non Existent Widget */
#define XsENOPIX	1003	/* Non Existent Pixmap */
#define XsENOFONT	1004	/* Non Existent Font */
#define XsENOCOLOR	1005	/* Non Existent Color */
#define XsEINCRES	1006	/* Incomplete Resource Settings */
#define XsENOMEM	1007	/* Error Allocating Memory */
#define XsEINVWT	1008	/* Invalid Widget Type (Class) */
#define XsENOPAR	1009	/* Invalid Parameter */
#define XsEARGT		1010	/* Invalid Argument Type */
#define XsENOCSTR	1011	/*  Error in Compound String Mngt */
#define XsENOCB		1012	/* Null Callback Function */
#define XsENOCBVAL	1013	/* Null Value for Callback Client Data */
#define XsEINVCBVAL	1014	/* Invalid Callback Client Data Type */

/* Widget Descriptor Struct Template */

typedef struct {
	char		*wName;         /* Name of the Widget */
#define WDEND		(char *)NULL
	Widget		wPnt;           /* pointer alla Widget */
	char		*pName;         /* Parent Name */
	unsigned int	wFlag;          /* Widget Flags */
	Widget		(*wCreate)();   /* Widget Creation Func */
	Arg		*pArg;          /* pointer agli argomenti */
	Cardinal	nArg;           /* numero argomenti */
	unsigned int	uData;          /* User Data */
} XsWdesc;

#define NULLWFP		(Widget (*)())0
#define XsNULLWFP	(Widget (*)())0

/* Callback Descriptor Template */

typedef struct {
	char		*wName;         /* widget name  in wDesc */
#define CBEND		(char *)NULL
	String		cbName;         /* Callback name */
	void		(*cbp)();       /* Callback procedure */
	int		dType;          /* data type */
#define XsCBNULL	0
#define XsCBVAL		1
#define XsCBPNT		2
#define XsCBWNAME	3
#define XsCBDIR		4
	void		*cbData;        /* Callback User Data */
} XsCbDesc;


/* Resources Argument List Descriptor Template */

typedef struct {
	char		*wName;
#define ADEND		(char *)NULL
#ifdef SYSV
	void		*argName;
#else
	char		*argName;
#endif
	int		argType;
#define XsPointerArgType	1
#define XsValueArgType		2
#define XsCstringArgType	3
#define XsColorArgType		4
#define XsFontArgType		5
#define XsWNameArgType		6
#define XsPixmapArgType		7
	void		*value;
} XsArgDesc;

/* Functions Declarations */
int		XsLibInit(unsigned char, char *, char *);
int		XsCbDescInstall(XsWdesc *, XsCbDesc *);
int		XsWidgetDescCreate(XsWdesc *,Widget,XsArgDesc *);
int		XsParseWidgetName(XsWdesc *, char *);
int		XsMatchWidget(XsWdesc *,Widget);
int		XsSetForeground(Widget, char *);
int		XsSetBackground(Widget, char *);
int		XsGetColorPixel(char *, Pixel *);
int		XsShowWidget(XsWdesc *, char *);
int		XsHideWidget(XsWdesc *, char *);
int		XsErrorPopup(XsWdesc *,char *);
int		XsWarningPopup(XsWdesc *,char *);
int		XsInfoPopup(XsWdesc *,char *);
char		*XsStrErr(int);
int		XsHelpDisplay(XsWdesc *,char *,char *);

/* Callbacks */
void	XsShowWCB(Widget, Widget, XmAnyCallbackStruct *);
void	XsHideWCB(Widget, Widget, XmAnyCallbackStruct *);
void	XsHelpCB(Widget, XsWdesc *, XmAnyCallbackStruct *);

/* Convenience Functions for Athena Widget Set */

/* boxWidgetClass */
Widget XsAwCreateBox(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwBox(Widget w);

/* commandWidgetClass */
Widget XsAwCreateCommand(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwCommand(Widget w);

/* dialogWidgetClass */
Widget XsAwCreateDialog(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwDialog(Widget w);

/* formWidgetClass */
Widget XsAwCreateForm(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwForm(Widget w);

/* gripWidgetClass */
Widget XsAwCreateGrip(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwGrip(Widget w);

/* labelWidgetClass */
Widget XsAwCreateLabel(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwLabel(Widget w);

/* listWidgetClass */
Widget XsAwCreateList(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwList(Widget w);

/* menuButtonWidgetClass */
Widget XsAwCreateMenuButton(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwMenuButton(Widget w);

/* panedWidgetClass */
Widget XsAwCreatePaned(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwPaned(Widget w);

/* scrollbarWidgetClass */
Widget XsAwCreateScrollbar(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwScrollbar(Widget w);

/* simpleMenuWidgetClass */
Widget XsAwCreateSimpleMenu(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwSimpleMenu(Widget w);

/* smeObjectClass */
Widget XsAwCreateSme(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwSme(Widget w);

/* smeBSBObjectClass */
Widget XsAwCreateSmeBSB(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwSmeBSB(Widget w);

/* smeLineWidgetClass */
Widget XsAwCreateSmeLine(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwSmeLine(Widget w);

/* stripChartWidgetClass */
Widget XsAwCreateStripChart(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwStripChart(Widget w);

/* textWidgetClass */
Widget XsAwCreateText(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwText(Widget w);

/* asciiTextWidgetClass */
Widget XsAwCreateAsciiText(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwAsciiText(Widget w);

/* toggleWidgetClass */
Widget XsAwCreateToggle(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwToggle(Widget w);

/* viewportWidgetClass */
Widget XsAwCreateViewport(Widget,char *,ArgList,Cardinal);
Boolean XsIsAwViewport(Widget w);

#ifdef __STDC__
int		XsTscanf(Widget,char const *,...);
int		XsTNscanf(XsWdesc *,char *,char const *,...);
int		XsTprintf(Widget,char const *,...);
int		XsTNprintf(XsWdesc *,char *,char const *,...);
int		XsLprintf(Widget,char const *,...);
int		XsLNprintf(XsWdesc *,char *,char const *,...);
int		XsVaErrPopup(XsWdesc *,char const *,...);
int		XsVaWarnPopup(XsWdesc *,char const *,...);
int		XsVaInfoPopup(XsWdesc *,char const *,...);
#else 
int		XsTscanf(Widget,char const *,va_dcl);
int		XsTNscanf(XsWdesc *,char *,char const *,va_dcl);
int		XsTprintf(Widget,char const *,va_dcl);
int		XsTNprintf(XsWdesc *,char *,char const *,va_dcl);
int		XsLprintf(Widget,char const *,va_dcl);
int		XsLNprintf(XsWdesc *,char *,char const *,va_dcl);
int		XsVaErrPopup(XsWdesc *,char const *,va_dcl);
int		XsVaWarnPopup(XsWdesc *,char const *,va_dcl);
int		XsVaInfoPopup(XsWdesc *,char const *,va_dcl);
#endif
