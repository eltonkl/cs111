#ifndef _GNU_SOURCE
#define _GNU_SOURCE


#include "addtest.h"
#include <pthread.h>

pthread_mutex_t mutexlock;

void add (long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if (opt_yield)
       pthread_yield();
    *pointer = sum;

}

void mutex_add (long long *pointer, long long value)
{
    pthread_mutex_lock(&mutexlock);
    long long sum = *pointer + value;
    *pointer = sum;
    pthread_mutex_unlock(&mutexlock);

}

void spin_add (long long *pointer, long long value)
{
    while(__sync_lock_test_and_set(&spinlock, 1))
        continue;
    long long sum = *pointer + value;
    if (opt_yield)
       pthread_yield();
    *pointer = sum;

    __sync_lock_release(&spinlock);
    

}

void swap_add (long long *pointer, long long value)
{
    int orig;
    int sum;
    do
    {
        orig = *pointer;
        if(opt_yield)
            pthread_yield();
        sum = orig + value;
    }
    while (__sync_val_compare_and_swap(pointer, orig, sum) != orig);

}

#endif  
