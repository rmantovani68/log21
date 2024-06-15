#ifndef __a_out_h
#define __a_out_h
/************************************************************
(C) Copyright 1987-1994
Lynx Real-Time Systems, Inc.
Los Gatos, CA
All rights reserved.

 File: a.out.h
$Date: 95/02/28 09:41:52 $
$Revision: 5.15 $
************************************************************/

#ifndef __ansidecl_lynx_h
#include <ansidecl_lynx.h>	/* For function prototyping */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	Header block
	Placed at the start of object and executable files
*/

struct exec {
	long a_magic;				/* Type of a.out file */
	unsigned long a_text;		/* Text segment size */
	unsigned long a_data;		/* Init. data segment size */
	unsigned long a_bss;		/* Uninitialized data segment size */
	unsigned long a_syms;		/* Symbol table size */
	unsigned long a_entry;		/* Entry point */
	unsigned long a_trsize;		/* Text relocation size */
	unsigned long a_drsize;		/* Data relocation size */
};

#define OMAGIC			0407	/* Object (impure) file */
#define ZMAGIC			0413	/* Executable file */
#define ZVMAGIC			0414	/* SYSTEM V executable (not COFF) */
#ifdef __mips__
#define IMAGIC			0310	/* Executable file with text stripped */
#else
#define IMAGIC			0410	/* Executable file with text stripped */
#endif
#define SPARC_MAGIC		0403	/* Sparc architecture tag in upper byte */
#define MIPSEBMAGIC		0x0160	/* MIPS big endian */
#define SMIPSEBMAGIC	0x6001
#define MIPSEBUMAGIC	0x0180

/*
	Macros to inspect header blocks for goodness, and to locate the
	various segments within the file
*/
#if defined(SPARC) || defined(__SPARC__)
/*
 * SPARC (actually SunOS and all compilers written for it)
 * puts extra data in the upper 2 bytes of the magic
 * number, indicating architecture, shared libs, and so on.
 */
#define a_info			a_magic
#if !defined (N_MAGIC)
#define N_MAGIC(exec)	((exec).a_info & 0xffff)
#endif

#if !defined (N_BADMAG)
#define N_BADMAG(x) \
	(N_MAGIC(x)!=OMAGIC && N_MAGIC(x)!=ZMAGIC && N_MAGIC(x)!=ZVMAGIC)
#endif

#if !defined (N_TXTOFF)
#define N_TXTOFF(x)		(N_MAGIC(x) == ZMAGIC ? 0 : sizeof(struct exec))
#endif

#if !defined (N_DATOFF)
#define N_DATOFF(x)		(N_TXTOFF(x) + (x).a_text)
#endif

#if !defined (N_TRELOFF)
#define N_TRELOFF(x)	(N_DATOFF(x) + (x).a_data)
#endif

#if !defined (N_DRELOFF)
#define N_DRELOFF(x)	(N_TRELOFF(x) + (x).a_trsize)
#endif

#if !defined (N_SYMOFF)
#define N_SYMOFF(x)		(N_DRELOFF(x) + (x).a_drsize)
#endif

#else /* SPARC || __SPARC__ */

#define N_BADMAG(x) \
	(((x).a_magic)!=OMAGIC && ((x).a_magic)!=ZMAGIC && ((x).a_magic)!=ZVMAGIC)

#define N_OLDOFF(x) \
	(((x).a_magic==ZMAGIC || (x).a_magic==ZVMAGIC) ? 0 : sizeof(struct exec))

#define N_TXTOFF(x)		(sizeof(struct exec))

#define N_SYMOFF(x) \
	(N_OLDOFF(x) + (x).a_text+(x).a_data + (x).a_trsize+(x).a_drsize)

#endif /* SPARC || __SPARC__ */

/* Common to SPARC and non-SPARC */
#if !defined (N_STROFF)
#define N_STROFF(x)		(N_SYMOFF(x) + (x).a_syms)
#endif

/*
	relocation information
*/

struct relocation_info {
	unsigned long r_address;	/* address which is relocated */
#ifdef UNIX
	unsigned int
		r_symbolnum:24,			/* local symbol ordinal */
		r_pcrel:1,				/* was relocated pc relative already */
		r_length:2,				/* 0=byte, 1=word, 2=long */
		r_extern:1,				/* does not include value of sym */
		:4;						/* filler */
#define r_symbolnuma(x, v)	((x).r_symbolnum = (v))
#else
#if defined(M68K) || defined (__M68K__) || defined(M88K)
#ifdef IBITS32
	unsigned int
		r_symbolnum:24,			/* local symbol ordinal */
		r_pcrel:1,				/* was relocated pc relative already */
		r_length:2,				/* 0=byte, 1=word, 2=long */
		r_extern:1,				/* does not include value of sym */
		:4;						/* filler */
#define r_symbolnuma(x, v)	((x).r_symbolnum = (v))
#else
	union {
		unsigned long s1_symbolnum;
		struct {
			unsigned int s2_dum2;
			unsigned int
				s2_dum3:4,
				s2_extern:1,
				s2_length:2,
				s2_pcrel:1,
				s2_dum1:8;
		} r_scum2;
	} r_scum1;
#define r_symbolnum		r_scum1.s1_symbolnum >> 8
#ifndef r_symbolnuma
#define r_symbolnuma(x, v)	((x).r_scum1.s1_symbolnum \
				= (((x).r_scum1.s1_symbolnum & 0xff) | ((long)(v) << 8)))
#else
#define r_asymbolnuma(x, v)	((x).r_scum1.s1_symbolnum \
				= (((x).r_scum1.s1_symbolnum & 0xff) | ((long)(v) << 8)))
#endif
#define r_pcrel			r_scum1.r_scum2.s2_pcrel
#define r_length		r_scum1.r_scum2.s2_length
#define r_extern		r_scum1.r_scum2.s2_extern
#endif /* IBITS32 for M68K */

#else
#if defined(I386) || defined(__I386__)

	unsigned int
		r_dummy:4,		/* filler */
		r_extern:1,		/* does not include value of sym */
		r_length:2,		/* 0=byte, 1=word, 2=long */
		r_pcrel:1,		/* was relocated pc relative already */
		r_symbolnum:24;	/* local symbol ordinal */
#define r_symbolnuma(x, v)	((x).r_symbolnum = (v))

#endif
#ifdef __mips__
	unsigned int
		r_symbolnum:24,			/* local symbol ordinal */
		r_pcrel:1,				/* was relocated pc relative already */
		r_length:2,				/* 0=byte, 1=word, 2=long */
		r_extern:1,				/* does not include value of sym */
		:4;						/* filler */
#define r_symbolnuma(x, v)	((x).r_symbolnum = (v))
#endif
#if defined(SPARC) || defined(__SPARC__)
enum reloc_type {
	RELOC_8,
	RELOC_16,
	RELOC_32,
	RELOC_DISP8,
	RELOC_DISP16,
	RELOC_DISP32,
	RELOC_WDISP30,
	RELOC_WDISP22,
	RELOC_HI22,
	RELOC_22,
	RELOC_13,
	RELOC_LO10,
	RELOC_SFA_BASE,
	RELOC_SFA_OFF13,
	RELOC_BASE10,
	RELOC_BASE13,
	RELOC_BASE22,
	RELOC_PC10,
	RELOC_PC22,
	RELOC_JMP_TBL,
	RELOC_SEGOFF16,
	RELOC_GLOB_DAT,
	RELOC_JMP_SLOT,
	RELOC_RELATIVE,
	NO_RELOC,
};

/*
 * Format of relocation data, used for SPARC machines only.
 */

struct reloc_info_sparc {
	unsigned long int r_address;	/* relocation addr (in segment) */
	unsigned int r_index:24;		/* segment or symbol index */
	unsigned int r_extern: 1;		/* if 0, r_index==SEG#; if 1, SYM idx */
	int: 2;
	enum reloc_type r_type: 5;		/* type of relocation to perform */
	long int r_addend;				/* relocation value addend */
};

#endif
#endif /* M68K */
#endif /* UNIX */
};

#define _FROM_AOUT
#include <nlist.h>
#undef _FROM_AOUT

/*
 * Ok.  Following are the relocation information macros.  If your
 * system cannot use the default set (below), you must define all of these:
 *
 *   relocation_info: This must be typedef'd (or #define'd) to the type
 * of structure that is stored in the relocation info section of your
 * a.out files.  Often this is defined in the a.out.h for your system.
 *
 *   RELOC_ADDRESS (rval): Offset into the current section of the
 * <whatever> to be relocated.  *Must be an lvalue*.
 *
 *   RELOC_EXTERN_P (rval):  Is this relocation entry based on an
 * external symbol (1), or was it fully resolved upon entering the
 * loader (0) in which case some combination of the value in memory
 * (if RELOC_MEMORY_ADD_P) and the extra (if RELOC_ADD_EXTRA) contains
 * what the value of the relocation actually was.  *Must be an lvalue*.
 *
 *   RELOC_TYPE (rval): For a non-external relocation, this is the
 * segment to relocate for.  *Must be an lvalue.*
 *
 *   RELOC_SYMBOL (rval): For an external relocation, this is the
 * index of its symbol in the symbol table.  *Must be an lvalue*.
 *
 *   RELOC_MEMORY_ADD_P (rval): This should be 1 if the final
 * relocation value output here should be added to memory; 0, if the
 * section of memory described should simply be set to the relocation
 * value.
 *
 *   RELOC_MEMORY_ADD_P (rval): If this is nonzero, the value previously
 * present in the memory location to be relocated is *added*
 * to the relocation value, to produce the final result.
 * Otherwise, the relocation value is stored in the memory location
 * and the value previously found there is ignored.
 * By default, this is always 1.
 *
 *   RELOC_MEMORY_SUB_P (rval): If this is nonzero, the value previously
 * present in the memory location to be relocated is *subtracted*
 * from the relocation value, to produce the final result.
 * By default, this is always 0.
 *
 *   RELOC_ADD_EXTRA (rval): (Optional) This macro, if defined, gives
 * an extra value to be added to the relocation value based on the
 * individual relocation entry.  *Must be an lvalue if defined*.
 *
 *   RELOC_PCREL_P (rval): True if the relocation value described is
 * pc relative.
 *
 *   RELOC_VALUE_RIGHTSHIFT (rval): Number of bits right to shift the
 * final relocation value before putting it where it belongs.
 *
 *   RELOC_TARGET_SIZE (rval): log to the base 2 of the number of
 * bytes of size this relocation entry describes; 1 byte == 0; 2 bytes
 * == 1; 4 bytes == 2, and etc.  This is somewhat redundant (we could
 * do everything in terms of the bit operators below), but having this
 * macro could end up producing better code on machines without fancy
 * bit twiddling.  Also, it's easier to understand/code big/little
 * endian distinctions with this macro.
 *
 *   RELOC_TARGET_BITPOS (rval): The starting bit position within the
 * object described in RELOC_TARGET_SIZE in which the relocation value
 * will go.
 *
 *   RELOC_TARGET_BITSIZE (rval): How many bits are to be replaced
 * with the bits of the relocation value.  It may be assumed by the
 * code that the relocation value will fit into this many bits.  This
 * may be larger than RELOC_TARGET_SIZE if such be useful.
 *
 *
 *		Things I haven't implemented
 *		----------------------------
 *
 *    Values for RELOC_TARGET_SIZE other than 0, 1, or 2.
 *
 *    Pc relative relocation for External references.
 *
 *
 */

#if defined(SPARC) || defined(__SPARC__)
/* Sparc (Sun 4) macros */
#undef relocation_info
#define relocation_info					reloc_info_sparc
#define RELOC_ADDRESS(r)				((r).r_address)
#define RELOC_EXTERN_P(r)				((r).r_extern)
#define RELOC_TYPE(r)					((r).r_index)
#define RELOC_SYMBOL(r)					((r).r_index)
#define RELOC_MEMORY_SUB_P(r)			0
#define RELOC_MEMORY_ADD_P(r)			0
#define RELOC_ADD_EXTRA(r)				((r).r_addend)
#define RELOC_PCREL_P(r) \
	((r).r_type >= RELOC_DISP8 && (r).r_type <= RELOC_WDISP22)
#define RELOC_VALUE_RIGHTSHIFT(r)		(reloc_target_rightshift[(r).r_type])
#define RELOC_TARGET_SIZE(r)			(reloc_target_size[(r).r_type])
#define RELOC_TARGET_BITPOS(r)			0
#define RELOC_TARGET_BITSIZE(r)			(reloc_target_bitsize[(r).r_type])

/*
	Note that these are very dependent on the order of the enums in enum
	reloc_type (in a.out.h); if they change the following must be changed.
	Also note that the last few may be incorrect; I have no information.
*/
static int reloc_target_rightshift[] = {
	0, 0, 0, 0, 0, 0, 2, 2, 10, 0, 0, 0, 0, 0, 0,
};
static int reloc_target_size[] = {
	0, 1, 2, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};
static int reloc_target_bitsize[] = {
	8, 16, 32, 8, 16, 32, 30, 22, 22, 22, 13, 10, 32, 32, 16,
};
#endif


#ifdef __cplusplus
}
#endif

#endif /* __a_out_h */
