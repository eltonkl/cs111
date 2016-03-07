#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    SortedListElement_t* cur = list;
    if (!cur || !element)
        return;
    while (cur->next)
    {
        if (strcmp(cur->next->key, element->key) > 0)
            break;
        else
            cur = cur->next;
    }

    if (!cur->next)
    {
        list->prev = element;
        cur->next = element;
        element->prev = cur;
        element->next = NULL;
    }
    else
    {
        cur->next->prev = element;
        element->next = cur->next;
        element->prev = cur;
        cur->next = element;
    }

    if (opt_yield & INSERT_YIELD)
       ;
}

int SortedList_delete(SortedListElement_t *element)
{
   if (opt_yield & DELETE_YIELD)
       ;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
   if (opt_yield & SEARCH_YIELD)
       ;
}

int SortedList_length(SortedList_t *list)
{
   if (opt_yield & SEARCH_YIELD)
       ;
}
