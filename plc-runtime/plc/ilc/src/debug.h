/*
* @(#) debug.h 1.0 Fri Jul 12 18:03:23 MET DST 1996
*
*  Questo file contiene funzioni utili a "capirci qualcosa".
*/

#ifndef _DEBUG_H_

#ifdef DEBUG

void db_printf(const char *format,...);

void db_putval(LPVAL op);

void db_printval(LPVAL op);

void db_dump_arglist(type_t * tp);

void db_dump_type(type_t * tp);

void db_dump_types(symbol_t * sym);

void db_dump_var(symbol_t * sym);

void db_dump_context(context_t * context);

void db_dump_context_chain(void);

#else

#define db_printf()
#define db_printval()
#define db_dump_arglist()
#define db_dump_type()
#define db_dump_types()
#define db_dump_var()
#define db_dump_context()
#define db_dump_context_chain()

#endif

#define _DEBUG_H_

#endif

