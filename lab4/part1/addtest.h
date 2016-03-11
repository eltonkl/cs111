#ifndef ADD_H
#define ADD_H

#include <pthread.h>

int nthreads;
int niterations;
volatile int spinlock;
extern pthread_mutex_t mutexlock;
int sync;
int opt_yield;
extern long long counter;

void add (long long *pointer, long long value);
void mutex_add (long long *pointer, long long value);
void spin_add (long long *pointer, long long value);
void swap_add (long long *pointer, long long value);
#endif
