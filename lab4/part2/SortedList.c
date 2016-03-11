#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    SortedListElement_t* cur = list;
    if (!cur || !element)
        return;
    while (cur->next != list)
    {
        if (strcmp(cur->next->key, element->key) > 0)
            break;
        else
            cur = cur->next;
    }

    if (opt_yield & INSERT_YIELD)
       pthread_yield();

    cur->next->prev = element;
    element->next = cur->next;
    element->prev = cur;
    cur->next = element;
}

int SortedList_delete(SortedListElement_t *element)
{
    if (!element)
        return 1;
    else if (!element->next || element->next->prev != element)
        return 1;
    else if (!element->prev || element->prev->next != element)
        return 1;

    element->next->prev = element->prev;
    if (opt_yield & DELETE_YIELD)
       pthread_yield();
    element->prev->next = element->next;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t* cur = list;
    if (!cur || !key)
        return NULL;

    while (cur->next != list)
    {
        if (opt_yield & SEARCH_YIELD)
            pthread_yield();
        if (cur->next && strcmp(cur->next->key, key) == 0)
            return cur->next;
        else if (cur->next)
            cur = cur->next;
        else
            break;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
    int length = 0;
    SortedListElement_t* cur = list;
    while (cur->next != list)
    {
        length++;
        if (opt_yield & SEARCH_YIELD)
            pthread_yield();
        if (cur->next->prev != cur || cur->prev->next != cur)
            return -1;
        cur = cur->next;
    }
    return length;
}
