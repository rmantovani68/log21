#ifndef __in_h
#define __in_h
/************************************************************
(C) Copyright 1987-1994
Lynx Real-Time Systems, Inc., Los Gatos, CA
All rights reserved.

 File: in.h
$Date: 94/06/17 19:52:48 $
$Revision: 5.4 $
************************************************************/

#include <ansidecl_lynx.h>  /* _AP wrapper for ANSI prototypes */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)in.h	7.6 (Berkeley) 6/29/88
 */

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */

#define RN_MASK		0x0f	/* Mask for right nibble in a byte */
#define LN_MASK		0xf0	/* Mask for left nibble in a byte */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/*
 * Link numbers
 */
#define	IMPLINK_IP		155
#define	IMPLINK_LOWEXPER	156
#define	IMPLINK_HIGHEXPER	158

/*
 * Internet address (a structure for historical reasons)
 *      This definition contains obsolete fields for compatibility
 *      with SunOS 3.x and 4.2bsd.  The presence of subnets renders
 *      divisions into fixed fields misleading at best.  New code
 *      should use only the s_addr field.
 */
struct in_addr {
        union {
                struct { u_char s_b1, s_b2, s_b3, s_b4; } S_un_b;
                struct { u_short s_w1, s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
#define s_addr  S_un.S_addr             /* should be used for all code */
#define s_host  S_un.S_un_b.s_b2        /* OBSOLETE: host on imp */
#define s_net   S_un.S_un_b.s_b1        /* OBSOLETE: network */
#define s_imp   S_un.S_un_w.s_w2        /* OBSOLETE: imp */
#define s_impno S_un.S_un_b.s_b4        /* OBSOLETE: imp # */
#define s_lh    S_un.S_un_b.s_b3        /* OBSOLETE: logical host */
};


/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((long)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((long)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((long)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((long)(i) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((long)(i) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(i)		(((long)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(u_long)0x00000000
#define	INADDR_BROADCAST	(u_long)0xffffffff	/* must be masked */
#ifndef KERNEL
#define	INADDR_NONE		0xffffffff		/* -1 return */
#endif

#define	IN_LOOPBACKNET		127			/* official! */

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS	1		/* set/get IP per-packet options */

#ifdef KERNEL
extern	struct domain inetdomain;
extern	struct protosw inetsw[];
#ifndef SPARC
struct	in_addr in_makeaddr();
#else
u_long in_makeaddr();
#endif
u_long	in_netof(), in_lnaof();
#endif

unsigned short htons _AP((unsigned int)), ntohs _AP((unsigned int));
unsigned long htonl _AP((long unsigned int)), ntohl _AP((long unsigned int));

#ifdef __cplusplus
}
#endif

#endif /* __in_h */
