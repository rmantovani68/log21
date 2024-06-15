/*
* linklist.c
* gestione linked list
*/

#include <stdlib.h>
#include "linklist.h"

#define  LINK_PTR(p)  ((LINK_ITEM *)p)

void list_reset(LINKED_LIST *list)
{
	list->n_link=0;
	list->last=0x0;
	list->selected=0x0;
}

void list_free(LINKED_LIST *list)
{
	LINK_ITEM *p ;

	while((p=link_pop(list))){
		free(p);
	}
}

void *link_first(LINKED_LIST *list)
{
	if(list->last == 0)  return 0 ;
	return (void *)list->last->n ;
}

void *link_next(LINKED_LIST *list, void *link)
{
	if(link == (void *)list->last)  return 0 ;
	if(link == 0)  return link_first(list) ;

	return (void *)(LINK_PTR(link)->n) ;
}

void *link_prev(LINKED_LIST *list, void *link)
{
	if(link == 0)  return (void *)list->last ;
	if((void *)list->last->n == link)  return 0 ;

	return (void *)(LINK_PTR(link)->p) ;
}

void *link_last(LINKED_LIST *list)
{
	return (void *)list->last ;
}

void link_add_before(LINKED_LIST *list, void *anchor, void *new_link)
{
	if(list->last == 0) {
		list->last = LINK_PTR(new_link);
		LINK_PTR(new_link)->p = LINK_PTR(new_link);
		LINK_PTR(new_link)->n = LINK_PTR(new_link);
	} else {
		LINK_PTR(new_link)->n = LINK_PTR(anchor);
		LINK_PTR(new_link)->p = LINK_PTR(anchor)->p ;
		LINK_PTR(anchor)->p->n= LINK_PTR(new_link);
		LINK_PTR(anchor)->p   = LINK_PTR(new_link);
	}

	list->n_link++ ;

	return ;
}


void link_add_after(LINKED_LIST *list, void *anchor, void *new_link)
{
	if(list->last == 0) {
		list->last = LINK_PTR(new_link);
		LINK_PTR(new_link)->p = LINK_PTR(new_link);
		LINK_PTR(new_link)->n = LINK_PTR(new_link);
	} else {
		LINK_PTR(new_link)->p =  LINK_PTR(anchor);
		LINK_PTR(new_link)->n =  LINK_PTR(anchor)->n ;
		LINK_PTR(anchor)->n->p=  LINK_PTR(new_link);
		LINK_PTR(anchor)->n   =  LINK_PTR(new_link);
		if(anchor==(void *)list->last) list->last=LINK_PTR(new_link);
	}

	list->n_link++ ;

	return ;
}

void link_add(LINKED_LIST *list, void *new_link)
{
	link_add_after(list, list->last, new_link) ;
}

int link_remove(LINKED_LIST *list, void *remove_link)
{
	LINK_ITEM *link_on ;

	if (remove_link == 0)  return 0;

	/* 
	* Make sure the link being removed is on the linked list ! 
	*/
	for (link_on = 0; link_on =  (LINK_ITEM *) link_next(list,link_on);)
		if((void *)link_on == remove_link)  break ;

	if((void *)link_on != remove_link)
		return(0);

	if(list->selected == remove_link) {
		list->selected =  (void *)(LINK_PTR(remove_link)->n) ;
		if(list->selected == remove_link)  list->selected =  0 ;
	}

	LINK_PTR(remove_link)->p->n = LINK_PTR(remove_link)->n ;
	LINK_PTR(remove_link)->n->p = LINK_PTR(remove_link)->p ;

	if(remove_link == (void *)list->last) {
		if(list->last->p == list->last)
			list->last =  0 ;
		else
			list->last =  list->last->p ;
	}

	list->n_link-- ;

	return 1;
}

void *link_pop(LINKED_LIST *list)
{
	LINK_ITEM *p ;

	p = list->last ;
	link_remove(list, list->last) ;
	return (void *)p ;
}


