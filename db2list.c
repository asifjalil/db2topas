/*****************************************************************************
*                                                                            *
*  ----------------------------- db2list.c --------------------------------  *
*                                                                            *
*****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "db2list.h"
#include "db2topasutil.h"

/*****************************************************************************
*                                                                            *
*  ---------------------------- Private Functions -------------------------  *
*                                                                            *
*****************************************************************************/
void swap (DB2ListElmt *element1, DB2ListElmt *element2) ;

/*****************************************************************************
*                                                                            *
*  ---------------------------- db2list_init ------------------------------  *
*                                                                            *
*****************************************************************************/

void  db2list_init(DB2List *list, int (*match)(const void *key1, const void *key2),
void (*destroy)(void *data)
, SnapReqType snap_req_type
)
{

/*****************************************************************************
*                                                                            *
*  Initialize the list.                                                      *
*                                                                            *
*****************************************************************************/

  list->snap_req_type = snap_req_type;
  list->size = 0;
  list->match = match;
  list->destroy = destroy;
  list->head = NULL;
  list->tail = NULL;
  list->highlighted_element = NULL;

  return;

}


/*****************************************************************************
*                                                                            *
*  -------------------------- db2list_destroy -----------------------------  *
*                                                                            *
*****************************************************************************/

void  db2list_destroy(DB2List *list)
{

  void  *data;

/*****************************************************************************
*                                                                            *
*  Remove each element.                                                      *
*                                                                            *
*****************************************************************************/

  while (db2list_size(list) > 0) {

    if (db2list_remove(list, db2list_tail(list), (void **) & data) == 0 && list->
        destroy != NULL) {

      /***********************************************************************
      *                                                                      *
      *  Call a user-defined function to free dynamically allocated data.    *
      *                                                                      *
      ***********************************************************************/

      list->destroy(data);

    }

  }

  memset(list, 0, sizeof(DB2List));

  return;

}


/*****************************************************************************
*                                                                            *
*  -------------------------- db2list_cleanup -----------------------------  *
*                                                                            *
*****************************************************************************/
void  db2list_cleanup(DB2List *list)
{

  void  *data;
  DB2ListElmt     * cur_element;
  DB2ListElmt     * old_element;

  for (cur_element = db2list_head(list); cur_element != NULL; ) {

    if (cur_element->delete == TRUE) {
      old_element = cur_element;
      cur_element = db2list_next(cur_element);
      if (db2list_remove(list, old_element, (void **) & data) == 0 && list->
          destroy != NULL)
        list->destroy(data);
    } else
      cur_element = db2list_next(cur_element);
  }


  return;
}


/*****************************************************************************
*                                                                            *
*---------------------------- db2list_ins_next ----------------------------  *
*                                                                            *
*****************************************************************************/

int  db2list_ins_next(DB2List *list, DB2ListElmt *element, const void *data)
{

  DB2ListElmt          * new_element;

/*****************************************************************************
*                                                                            *
*  Do not allow a NULL element unless the list is empty.                     *
*                                                                            *
*****************************************************************************/

  if (element == NULL && db2list_size(list) != 0)
    return - 1;

/*****************************************************************************
*                                                                            *
*  Allocate storage for the element.                                         *
*                                                                            *
*****************************************************************************/

  if ((new_element = (DB2ListElmt * )malloc(sizeof(DB2ListElmt))) == NULL)
    return - 1;

/*****************************************************************************
*                                                                            *
*  Insert the new element into the list.                                     *
*                                                                            *
*****************************************************************************/

  new_element->data = (void *)data;
  new_element->delete = FALSE;
  new_element->highlight = FALSE;

  if (db2list_size(list) == 0) {

   /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is empty.                               *
   *                                                                         *
   **************************************************************************/

    list->head = new_element;
    list->head->prev = NULL;
    list->head->next = NULL;
    list->tail = new_element;

  } 
  else {

  /**************************************************************************
   *                                                                         *
   *  Handle insertion when the list is not empty.                           *
   *                                                                         *
   **************************************************************************/

    new_element->next = element->next;
    new_element->prev = element;

    if (element->next == NULL)
      list->tail = new_element;
    else
      element->next->prev = new_element;

    element->next = new_element;

  }

/*****************************************************************************
*                                                                            *
*  Adjust the size of the list to account for the inserted element.          *
*                                                                            *
*****************************************************************************/

  list->size++;

  return 0;

}


/*****************************************************************************
*                                                                            *
*  ----------------------------- db2list_remove -----------------------------  *
*                                                                            *
*****************************************************************************/

int  db2list_remove(DB2List *list, DB2ListElmt *element, void **data) 
{

/*****************************************************************************
*                                                                            *
*  Do not allow a NULL element or removal from an empty list.                *
*                                                                            *
*****************************************************************************/

  if (element == NULL || db2list_size(list) == 0)
    return - 1;

/*****************************************************************************
*                                                                            *
*  Remove the element from the list.                                         *
*                                                                            *
*****************************************************************************/

  *data = element->data;
   if (element->highlight == TRUE)
     list->highlighted_element = NULL;

  if (element == list->head) {

   /**************************************************************************
   *                                                                         *
   *  Handle removal from the head of the list.                              *
   *                                                                         *
   **************************************************************************/

    list->head = element->next;

    if (list->head == NULL)
      list->tail = NULL;
    else 
      element->next->prev = NULL;

  } 
      else {

   /**************************************************************************
   *                                                                         *
   *  Handle removal from other than the head of the list.                   *
   *                                                                         *
   **************************************************************************/

    element->prev->next = element->next;

    if (element->next == NULL)
      list->tail = element->prev;
    else
      element->next->prev = element->prev;

  }

/*****************************************************************************
*                                                                            *
*  Free the storage allocated by the abstract data type.                     *
*                                                                            *
*****************************************************************************/

  free(element);

/*****************************************************************************
*                                                                            *
*  Adjust the size of the list to account for the removed element.           *
*                                                                            *
*****************************************************************************/

  list->size--;

  return 0;

}


/*****************************************************************************
*                                                                            *
*  --------------------------- db2list_lookup -----------------------------  *
*                                                                            *
*****************************************************************************/

DB2ListElmt  *db2list_lookup(const DB2List *list, void *data) 
{

  DB2ListElmt * element;

  for (element = db2list_head(list); element != NULL; element =   
      db2list_next(element)) {
    if (db2list_data(element) != NULL 
         && list->match(db2list_data(element), data) == 0) {
      element->delete = FALSE;
      return element;
    }
  }

  return NULL;
} /* db2list_lookup */




/*****************************************************************************
*                                                                            *
*  --------------------- db2list_check_highlight --------------------------  *
*                                                                            *
*****************************************************************************/
void  db2list_check_highlight(Header *header, DB2List *list 
, int (*compare) (const void *, const void *) 
)
{

  DB2ListElmt * element;
  if (db2list_size(list) == 0 || list->highlighted_element != NULL)
    return;

  /**************************************************************************/
  /* possibly highlight one of the list elements                            */
  /**************************************************************************/
  if (compare != NULL) {
    for (element = db2list_head(list); element != NULL; 
        element = db2list_next(element)) {
      if ( compare(header, db2list_data(element)) == 0) {
        element->highlight = TRUE;
        list->highlighted_element = element;
        return;
      }
    } /*for*/

  }

  (db2list_head(list))->highlight = TRUE;
  list->highlighted_element = db2list_head(list);

  return ;
}


/*****************************************************************************
*                                                                            *
*  --------------------- db2list_sort -------------------------------------  *
*                                                                            *
*****************************************************************************/
void db2list_sort(DB2List *list, int (*compare) (const void *, const void *))
{

  DB2ListElmt *left, *right, *key;
  if (list == NULL )
    return; 
  else if (db2list_size(list) < 2 )
    return; 

  /***********************************************************
  * db2list_sort uses insertion sort
  * -first for loop moves down (right) the list using a key
  * -a second for loop places the key
  *  in the sorted pile left of the list
  ***********************************************************/
  
  for (right = (db2list_head(list))->next; right != NULL; right = db2list_next(right) ) {
    key = right;
    for(left = db2list_prev(right); 
      left != NULL && compare(db2list_data(left),db2list_data(key)) > 0;
      left = db2list_prev(left) ) {
      swap(left, key);

      /* update pointer to the highlighted element */ 
      if (left->highlight == TRUE)
        list->highlighted_element = left;
      else if (key->highlight == TRUE)
        list->highlighted_element = key;

      /* after swap smaller/bigger value moved from key to left
         adjust key so it follows the smaller/bigger value */
      key = left;
    }

  }
  return;
}
/*****************************************************************************
*                                                                            *
*  --------------------- swap ---------------------------------------------  *
*                                                                            *
*****************************************************************************/
void swap (DB2ListElmt *element1, DB2ListElmt *element2) 
{

  void *data1 = db2list_data( element1);
  void *data2 = db2list_data( element2);

  Boolean highlight1 = ( element1)->highlight;
  Boolean highlight2 = ( element2)->highlight;
  
  /* swap the data */
  db2list_data( element1) = data2;
  db2list_data( element2) = data1;

  /* swap highlight indicator */
  ( element1)->highlight = highlight2;
  ( element2)->highlight = highlight1;
  
  return;
}
