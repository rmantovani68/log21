#ifndef __coff_h
#define __coff_h
/************************************************************
(C) Copyright 1987-1995
Lynx Real-Time Systems, Inc.
San Jose, CA
All rights reserved.

 File: coff.h
$Date: 97/11/07 17:26:48 $
$Revision: 5.36 $
************************************************************/

#ifdef Linux
#define __x86__ 1
#else
#ifndef __ansidecl_lynx_h
#include <ansidecl_lynx.h>	/* For function prototyping */
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* COFF Magic number */

/*
			BIG						LITTLE
	MCOFF	68k SysV *.o
	ZCOFF	68k exec	sparc		x86 exec
	COFF							x86 SysV *.o
	XCOFF	powerpc
*/

#define MCOFFMAGIC	0520		/* Motorola COFF Magic number:
									Used for Sys V objects and executables
									on M68K and for Lynx objects on M68K */
#define ZCOFFMAGIC	0415		/* Lynx COFF Magic number:
									Used for Lynx executables on I386 & M68K */
#define COFFMAGIC	0514		/* Interactive COFF Magic number:
									Used for Sys V objects and executables
									on I386 and for Lynx objects on I386 */
#define XCOFFMAGIC	0737		/* PowerPC XCOFF magic					*/

/************************************************************************/
/* Macros to support coff (non-sys V) magic numbers						*/
#if defined(__powerpc__)
#define COFFMAGICNUM XCOFFMAGIC
#else
#define COFFMAGICNUM ZCOFFMAGIC
#endif

#if defined(__x86__)
#define ISCOFFMAGIC(n) (COFFMAGICNUM==(n&0xffff))
#else
#define ISCOFFMAGIC(n) (COFFMAGICNUM==((n>>16)&0xffff))
#endif

#if !defined(IMAGIC)
#define IMAGIC 0410
#endif
/************************************************************************/


/* File header structure */
struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long			f_timdat;	/* time and date stamp */
	long			f_symptr;	/* file pointer to symbol table */
	long			f_nsyms;	/* no. of entries in sym table */
	unsigned short	f_opthdr;	/* size of optional header */
	unsigned short	f_flags;	/* flags - see below */
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

#define FILHDR		struct filehdr
#define FILHSZ		sizeof(FILHDR)	/* file header size */

/* Optional file header structure */

struct aouthdr {
	short		magic;		/* magic number */
	short		vstamp;		/* version stamp */
	long		tsize;		/* text size in bytes, padded */
	long		dsize;		/* initialized data size */
	long		bsize;		/* uninitialized data size */
	long		entry;		/* entry point */
	long		text_start;	/* base of text for this file */
	long		data_start;	/* base of data for this file */
#if defined(rs6000) || defined(__powerpc__)
	unsigned long	o_toc;			/* address of TOC */
	short			o_snentry;		/* section number for entry point */
	short			o_sntext;		/* section number for text */
	short			o_sndata;		/* section number for data */
	short			o_sntoc;		/* section number for toc */
	short			o_snloader;		/* section number for loader section */
	short			o_snbss;		/* section number for bss */
	short			o_algntext;		/* max alignment for text */
	short			o_algndata;		/* max alignment for data */
	char			o_modtype[2];	/* Module type field, 1R,RE,RO */
	char			o_resv1[2];		/* reserved field */
	unsigned long	o_maxstack;		/* max stack size allowed (bytes). */
	unsigned long	o_maxdata;		/* max data size allowed (bytes). */
	unsigned long	o_resv2[3];		/* reserved fields */
#endif
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

#define AOUTHDR		struct aouthdr
#define OPTHSZ		28		/* optional header size */

/* File header flags */

#define F_RELFLG	00001		/* Reloc info stripped from file */
#define F_EXEC		00002		/* File is executable */
#define F_LNNO		00004		/* Line no. info stripped */
#define F_LSYMS		00010		/* Local symbols stripped */
#define F_AR16WR	0000200		/* 16-bit reversed word */
#define F_AR32WR	0000400		/* 32-bit reversed word */

/* File header flags used on M68K (that are not included above) */

#define F_AR32W		0001000		/* 32-bit normal word (M68K ordering) */

/* Section header structure */

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to section raw data */
	long		s_relptr;	/* file ptr to relocation info */
	long		s_lnnoptr;	/* file ptr to line no. info */
	unsigned short	s_nreloc;	/* no. of relocation entries */
	unsigned short	s_nlnno;	/* no. of line no. entries */
	long		s_flags;	/* flags - see below */
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

#define SCNHDR	struct scnhdr
#define SCNHSZ	sizeof(SCNHDR)

/* Section header flags */

#define STYP_REG	0x00		/* regular section */
#define STYP_DSECT	0x01		/* dummy section */
#define STYP_NOLOAD	0x02		/* noload section */
#define STYP_GROUP	0x04		/* grouped section */
#define STYP_PAD	0x08		/* padding section */
#define STYP_COPY	0x10		/* copy section */
#define STYP_TEXT	0x20		/* executable text section */
#define STYP_DATA	0x40		/* initialized data section */
#define STYP_BSS	0x80		/* un-initialized data section */
#define STYP_INFO	0x200		/* comment section */
#define STYP_OVER	0x400		/* overlay section */
#define STYP_LIB	0x800		/* .lib section */

/* XCOFF defines */

#define STYP_EXCEPT     0x0100
#define STYP_LOADER     0x1000
#define STYP_DEBUG      0x2000
#define STYP_TYPCHK     0x4000
#define STYP_OVRFLO     0x8000


/* Relocation information structure */

struct reloc {
	long		r_vaddr;	/* virtual address of reference */
	long		r_symndx;	/* index into symbol table */
	union {
		unsigned short _r_type;	/* old style coff relocation type */
		struct {
			char	_r_rsize;	/* sign and reloc bit len */
			char	_r_rtype;	/* toc relocation type */
		} _r_r;
	} _r;
#ifdef __sparc__
#if defined(__GNUC_MINOR__)
	/* This is in sync with cygnus tools extended coff. */
	char r_spare[2];
	char r_offfset[4];
#endif
#endif /* __sparc__ */
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

#define r_type	_r._r_type	/* old style relocation - original name */
#define r_rsize	_r._r_r._r_rsize	/* extract sign and bit len */
#define r_rtype	_r._r_r._r_rtype	/* extract toc relocation type */

#if defined(TSUNAMI)
#define COFF_DATA_START 0x01000000
#endif

#define RELOC	struct reloc

#ifdef __sparc__
/* SPARC COFF relocation types */

enum sparc_reloc_type {
	R_SPARC_NONE = 0,
	R_SPARC_8,			R_SPARC_16,			R_SPARC_32,
	R_SPARC_DISP8,		R_SPARC_DISP16,		R_SPARC_DISP32,
	R_SPARC_WDISP30,	R_SPARC_WDISP22,
	R_SPARC_HI22,		R_SPARC_22,
	R_SPARC_13,			R_SPARC_LO10,
	R_SPARC_GOT10,		R_SPARC_GOT13,		R_SPARC_GOT22,
	R_SPARC_PC10,		R_SPARC_PC22,
	R_SPARC_WPLT30,
	R_SPARC_COPY,
	R_SPARC_GLOB_DAT,	R_SPARC_JMP_SLOT,
	R_SPARC_RELATIVE,
	R_SPARC_UA32,
	R_SPARC_max
};

#define RELSZ	sizeof(struct reloc)
#else
#define RELSZ	10
#endif /* __sparc__ */

#ifdef __powerpc__
#define R_LEN	0x1F		/* extract bit-length field */
#define R_SIGN	0x80		/* extract sign of relocation */
#define R_FIXUP	0x40		/* extract code-fixup bit */

#define RELOC_RLEN(x)	((x)._r._r_r._r_rsize & R_LEN)
#define RELOC_RSIGN(x)	((x)._r._r_r._r_rsize & R_SIGN)
#define RELOC_RFIXUP(x)	((x)._r._r_r._r_rsize & R_FIXUP)
#define RELOC_RTYPE(x)	((x)._r._r_r._r_rtype)
#endif

/* Relocation type flags */

#define R_ABS		0		/* absolute reference */
#define R_DIR16		01		/* direct, 16 bit reference */
#define R_REL16		02		/* PC-relative, 16 bit reference */
#define R_DIR32		06		/* direct, 32 bit reference */
#define R_PCRLONG	024		/* PC-relative, 32 bit reference */

/* Relocation type flags used on M68K (that are not included above) */

#define R_RELBYTE	017		/* M68K: direct, 8 bit reference */
#define R_RELWORD	020		/* M68K: direct, 16 bit reference */
#define R_RELLONG	021		/* M68K: direct, 32 bit reference */
#define R_PCRBYTE	022		/* M68K: PC-relative, 8 bit reference */
#define R_PCRWORD	023		/* M68K: PC-relative, 16 bit reference */

/*
 * POWER and PowerPC - relocation types
 */
#define R_POS	0x00	/* A(sym) Positive Relocation */
#define R_NEG	0x01	/* -A(sym) Negative Relocation */
#define R_REL	0x02	/* A(sym-*) Relative to self */
#define R_TOC	0x03	/* A(sym-TOC) Relative to TOC */
#define R_TRL	0x12	/* A(sym-TOC) TOC Relative indirect load. */
						/* modifiable instruction */
#define R_TRLA	0x13	/* A(sym-TOC) TOC Rel load address. modifiable inst */
#define R_GL	0x05	/* A(external TOC of sym) Global Linkage */
#define R_TCL	0x06	/* A(local TOC of sym) Local object TOC address */
#define R_RL	0x0C	/* A(sym) Pos indirect load. modifiable instruction */
#define R_RLA	0x0D	/* A(sym) Pos Load Address. modifiable instruction */
#define R_REF	0x0F	/* AL0(sym) Non relocating ref. No garbage collect */
#define R_BA	0x08	/* A(sym) Branch absolute. Cannot modify instruction */
#define R_RBA	0x18	/* A(sym) Branch absolute. modifiable instruction */
#define R_RBAC	0x19	/* A(sym) Branch absolute constant. modifiable instr */
#define R_BR	0x0A	/* A(sym-*) Branch rel to self. non modifiable */
#define R_RBR	0x1A	/* A(sym-*) Branch rel to self. modifiable instr */
#define R_RBRC	0x1B	/* A(sym-*) Branch absolute const. */
						/* modifiable to R_RBR */
#define R_RTB	0x04	/* A((sym-*)/2) RT IAR Rel Branch. non modifiable */
#define R_RRTBI	0x14	/* A((sym-*)/2) RT IAR Rel Br. modifiable to R_RRTBA */
#define R_RRTBA	0x15	/* A((sym-*)/2) RT absolute br. modifiable to R_RRTBI */

/* Line number info structure */

struct lineno {
	union {
		long		l_symndx;	/* symtbl index of func name */
		long		l_paddr;	/* paddr of line number */
	} l_addr;
	unsigned short	l_lnno;		/* line number */
#ifdef __sparc__
	char	l_pad[2];			/* padding for alignment */
#endif
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

#define LINENO	struct lineno

#ifdef __sparc__
#define LINESZ	sizeof(struct lineno)
#else
#define LINESZ	6
#endif

/* Symbol table structure */

#define SYMNMLEN	8

struct syment {
	union {
		char		_n_name[SYMNMLEN];		/* symbol name */
		struct {
			long	_n_zeroes;	/* 0 if name is in str table */
			long	_n_offset;	/* location in string table */
		} _n_n;
#ifndef __sparc__
		char		*_n_nptr[2];
#endif
	} _n;
	unsigned long	n_value;		/* value of symbol */
	short			n_scnum;		/* section number */
	unsigned short	n_type;			/* type - see below */
	char			n_sclass;		/* storage class - below */
	char			n_numaux;		/* no. of aux entries */
#ifdef __sparc__
	char			n_pad[2];		/* padding for alignment */
#endif
#ifdef __GNUC__
} __attribute__ ((packed));
#else
};
#endif

/*
	NOTE : the n_name defn conflicts with the n_name member defined
	in struct nlist in the file nlist.h
*/
/*
#define n_name		_n._n_name
*/
#define n_zeroes	_n._n_n._n_zeroes
#define n_offset	_n._n_n._n_offset
#define n_nptr		_n._n_nptr[1]

#define SYMENT		struct syment

#ifdef __sparc__
#define SYMESZ		sizeof(struct syment)
#else
#define SYMESZ		18
#endif

/* Symbol - storage class flags */

#define C_EFCN		-1		/* physical end of function */
#define C_NULL		0
#define C_AUTO		1		/* automatic variable */
#define C_EXT		2		/* external symbol */
#define C_STAT		3		/* static */
#define C_REG		4		/* register variable */
#define C_EXTDEF	5		/* external definition */
#define C_LABEL		6		/* label */
#define C_ULABEL	7		/* undefined label */
#define C_MOS		8		/* member of structure */
#define C_ARG		9		/* function argument */
#define C_STRTAG	10		/* structure tag */
#define C_MOU		11		/* member of union */
#define C_UNTAG		12		/* union tag */
#define C_TPDEF		13		/* type definition */
#define C_USTATIC	14		/* unitialized static */
#define C_ENTAG		15		/* enumeration tag */
#define C_MOE		16		/* member of enumeration */
#define C_REGPARM	17		/* register parameter */
#define C_FIELD		18		/* bit field */
#define C_BLOCK		100		/* beginning and end of block */
#define C_FCN		101		/* beginning and end of function */
#define C_EOS		102		/* end of structure */
#define C_FILE		103		/* file name */
#define C_LINE		104		/* used only by utility programs */
#define C_ALIAS		105		/* duplicated tag */
#define C_HIDDEN	106		/* like static */
/* The following defines are for XCOFF */
/* Caveat: s_class now should be unsigned */
#define C_HIDEXT	107
#define C_BINCL		108
#define C_EINCL		109
#define C_GSYM		128
#define C_LSYM		129
#define C_PSYM		130
#define C_RSYM		131
#define C_RPSYM		132
#define C_STSYM		133
#define C_TCSYM		134
#define C_BCOMM		135
#define C_ECOML		136
#define C_ECOMM		137
#define C_DECL		140
#define C_ENTRY		141
#define C_FUN		142
#define C_BSTAT		143
#define C_ESTAT		144


/* Section number field */

/*
	The defs below should have been N_*, but that would lead to
	a conflict with the defs in nlist.h
*/
#define SN_DEBUG	-2		/* debugging symbol */
#define SN_ABS		-1		/* absolute symbol */
#define SN_UNDEF	0		/* undefined external symbol */

/* Symbol fundamental types */

#define T_NULL		0		/* type not assigned */
#define T_ARG		1		/* function argument */
#define T_CHAR		2
#define T_SHORT		3
#define T_INT		4
#define T_LONG		5
#define T_FLOAT		6
#define T_DOUBLE	7
#define T_STRUCT	8
#define T_UNION		9
#define T_ENUM		10
#define T_MOE		11
#define T_UCHAR		12		/* unsigned char */
#define T_USHORT	13
#define T_UINT		14
#define T_ULONG		15

/* Symbol fundamental types used on M68K (that are not included above) */

#define T_VOID		1

/* Symbol derived types */

#define DT_NON		0		/* no derived type */
#define DT_PTR		1		/* pointer */
#define DT_FCN		2		/* function */
#define DT_ARY		3		/* array */


/* Auxilary entry structure */

#define DIMNUM		4	/* Number of dimensions stored in auxilary
							entry for array type variables */

#define FILNMLEN	14	/* File name length in Lynx - 14 chars */

union auxent {
	struct {
		long	x_tagndx;
		union {
			struct {
				unsigned short x_lnno;
				unsigned short x_size;
			} x_lnsz;
			long	x_fsize;
		} x_misc;
		union {
			struct {
				long	x_lnnoptr;
				long	x_endndx;
			} x_fcn;			/* Function */
			struct {
				unsigned short x_dimen[DIMNUM];
			} x_ary;			/* Array */
		} x_fcnary;			/* Function - Array */
		unsigned short	x_tvndx;
	} x_sym;				/* Symbol table related */

	struct {
		char	x_fname[FILNMLEN];
	} x_file;				/* File name */

	struct {
		long			x_scnlen;
		unsigned short	x_nreloc;
		unsigned short	x_nlinno;
	} x_scn;				/* Section */

	struct {
		long			x_tvfill;
		unsigned short	x_tvlen;
		unsigned short	x_tvran[2];
	} x_tv;					/* TV ? */

	struct {
		long			x_scnlen;
		long			x_parmhash;
		unsigned short	x_snhash;
		unsigned char	x_smtyp;
		unsigned char	x_smclas;
		long			x_stab;
		unsigned short	x_snstab;
	} x_csect;				/* C_EXT/C_HIDEXT csect (XCOFF?) */
#ifdef __sparc__
	char x_pad[20];			/* Total size including alignment padding */
#endif
};

#define x_name		x_file._x_name
#define x_nptr		x_file._x_xptr[1]
#define x_zeroes	x_file._x_x._x_zeroes
#define x_offset	x_file._x_x._x_offset

#define AUXENT	union auxent

/* x_csect.x_smtyp & 3 */
#define	XTY_ER		0
#define	XTY_SD		1
#define	XTY_LD		2
#define	XTY_CM		3

#ifdef __sparc__
#define AUXESZ		sizeof(union auxent)
#else
#define AUXESZ		18
#endif

/* Loader section is only for PPC XCOFF */
struct ldhdr {
	long			l_version;	/* Loader section version number */
	long			l_nsyms;	/* Qty of loader Symbol table entries */
	long			l_nreloc;	/* Qty of loader relocation table entries */
	unsigned long	l_istlen;	/* Length of loader import file id strings */
	long			l_nimpid;	/* Qty of loader import file ids. */
	unsigned long	l_impoff;	/* Offset to start of loader import */
								/* file id strings */
	unsigned long	l_stlen;	/* Length of loader string table */
	unsigned long	l_stoff;	/* Offset to start of loader string table */
};

struct ldsym {
	union {
		char		_l_name[SYMNMLEN];	/* Symbol name */
		struct {
			long	_l_zeroes;		/* offset if 0 */
			long	_l_offset;		/* offset into loader string */
		} _l_l;
		char		*_l_nptr[2];	/* allows for overlaying */
	} _l;
	unsigned long	l_value;	/* Address field */
	short			l_scnum;	/* Section number */
	char			l_smtype;	/* type and imp/exp/eps */
								/* 0	Unused */
								/* 1	Import */
								/* 2	Entry point */
								/* 3	Export */
								/* 4	Unused */
								/* 5-7	Symbol type */
	char			l_smclas;	/* storage class */
	long			l_ifile;	/* import file id */
								/* string offset */
	long			l_parm;		/* type check offset */
								/* into loader string */
};

struct ldrel {
	unsigned long	l_vaddr;	/* Address field */
	long			l_symndx;	/* Loader symbol table index of */
								/* reloc value to apply. This field */
								/* is zero based where 0,1,2 are */
								/* text,data,bss and 3 is the first */
								/* symbol entry from above */
	unsigned short	l_rtype;	/* relocation type */
	short			l_rsecnm;	/* section number being relocated */
								/* one based index in scnhdr table */
};
#define l_name		_l._l_name
#define l_nptr		_l._l_nptr[1]
#define l_zeroes	_l._l_l._l_zeroes
#define l_offset	_l._l_l._l_offset

/* Storage Mapping Class definitions: x_smclas field of x_csect */
/* READ ONLY CLASSES */
#define XMC_PR	0	/* Program Code */
#define XMC_RO	1	/* Read Only Constant */
#define XMC_DB	2	/* Debug Dictionary Table */
#define XMC_GL	6	/* Global Linkage (Interfile Interface Code) */
#define XMC_XO	7	/* Extended Operation (Pseudo Machine Instruction) */
#define XMC_SV	8	/* Supervisor Call */
#define XMC_TI	12	/* Traceback Index csect */
#define XMC_TB	13	/* Traceback table csect */
/* READ WRITE CLASSES */
#define XMC_RW	5	/* Read Write Data */
#define XMC_TC0	15	/* TOC Anchor for TOC Addressability */
#define XMC_TC	3	/* General TOC item */
#define XMC_TD	16	/* Scalar data item in the TOC */
#define XMC_DS	10	/* Descriptor csect */
#define XMC_UA	4	/* Unclassified - Treated as Read Write */
#define XMC_BS	9	/* BSS class (uninitialized static internal) */
#define XMC_UC	11	/* Un-named Fortran Common */

#define PPC_SHLIB_MAGIC 0x4c594e58	/* Magic number for PPC shared libs */ 
					/* Ascii for "LYNX" */
#ifdef __cplusplus
}
#endif

#endif /* __coff_h */
