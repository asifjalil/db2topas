/*****************************************************************************
*                                                                            *
*  ----------------------------- db2list.h --------------------------------  *
*                                                                            *
*****************************************************************************/

#ifndef DB2LIST_H
#define DB2LIST_H

#include <stdlib.h>
#include "db2topasutil.h"

/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked list elements.                       *
*                                                                            *
*****************************************************************************/

typedef struct DB2ListElmt_ {

void                 *data;
Boolean              delete;
Boolean              highlight; 
struct DB2ListElmt_  *prev;
struct DB2ListElmt_  *next;

} DB2ListElmt;

/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked lists.                               *
*                                                                            *
*****************************************************************************/

typedef struct DB2List_ {

int                   size;

int                   (*match)(const void *key1, const void *key2);
void                  (*destroy)(void *data);

DB2ListElmt          *head;
DB2ListElmt          *tail;
DB2ListElmt          *highlighted_element; /* element highlighted in curses */

SnapReqType          snap_req_type ; /* type of snapshot elements in the list */

} DB2List;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

void db2list_init(DB2List *list, int (*match)(const void *key1, const void *key2), 
  void (*destroy)(void *data), SnapReqType);

void db2list_destroy(DB2List *list);

void db2list_cleanup(DB2List *list);

void db2list_sort(DB2List *list, int (*compare)(const void *key1, const void *key2));

int db2list_ins_next(DB2List *list, DB2ListElmt *element, const void *data);

int db2list_ins_prev(DB2List *list, DB2ListElmt *element, const void *data);

int db2list_remove(DB2List *list, DB2ListElmt *element, void **data);

DB2ListElmt *db2list_lookup(const DB2List *list, void *data);

void db2list_check_highlight(Header *header, DB2List *list 
  ,int (*compare) (const void *, const void *)) ;

#define db2list_get_type(list) ((list)->snap_req_type)

#define db2list_size(list) ((list)->size)

#define db2list_head(list) ((list)->head)

#define db2list_tail(list) ((list)->tail)

#define db2list_set_highlight(element) ((element)->highlight = TRUE)

#define db2list_unset_highlight(element) ((element)->highlight = FALSE)

#define db2list_set_list_highlight(list,element) ((list)->highlighted_element = element)

#define db2list_get_highlighted_element(list) ((list)->highlighted_element)

#define db2list_is_head(element) ((element)->prev == NULL ? TRUE : FALSE)

#define db2list_is_tail(element) ((element)->next == NULL ? TRUE : FALSE)

#define db2list_data(element) ((element)->data)

#define db2list_next(element) ((element)->next)

#define db2list_prev(element) ((element)->prev)

#define db2list_upd_match(list, new_match) ( (list)->match = new_match)


#endif

