/*
* linklist.h
* include file per gestione linked list
*/

#ifndef _LINK_LIST_H_

typedef struct tagLINK_ITEM {
   struct tagLINK_ITEM *n;
	 struct tagLINK_ITEM *p;
} LINK_ITEM ;

typedef struct tagLINKED_LIST {
   LINK_ITEM *last;  /* The last Link */
   void *selected;
   int n_link ;      /* The number of links in the list */
} LINKED_LIST, PLINKED_LIST;

/*
* funzioni utilizzate
*/
void list_reset(LINKED_LIST *list);
void list_free(LINKED_LIST *list);
void *link_first(LINKED_LIST *list);
void *link_next(LINKED_LIST *list, void *link);
void *link_prev(LINKED_LIST *list, void *link);
void *link_last(LINKED_LIST *list);
void link_add_before(LINKED_LIST *list, void *anchor, void *new_link);
void link_add_after(LINKED_LIST *list, void *anchor, void *new_link);
void link_add(LINKED_LIST *list, void *new_link);
int link_remove(LINKED_LIST *list, void *remove_link);
void *link_pop(LINKED_LIST *list);

#define _LINK_LIST_H_
#endif /* _LINK_LIST_H_ */
