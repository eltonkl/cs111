#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "addtest.h"
#include <time.h>
#include <pthread.h>
#include <math.h>

const int long_options_offset = 1;
long long counter = 0;
//extern pthread_mutex_t mutexlock;

void fwrapper1(void *arg)
{
    (void) arg;
    int i;
    for (i = 0; i < niterations; i++)
    {
        add(&counter, 1);
    }

    int j;
    for (j = 0; j < niterations; j++)
    {
        add(&counter, -1);
    }
}

void fwrapper2(void *arg)
{
    (void) arg;
    int i;
    for (i = 0; i < niterations; i++)
    {
        mutex_add(&counter, 1);
    }

    int j;
    for (j = 0; j < niterations; j++)
    {
        mutex_add(&counter, -1);
    }
}

void fwrapper3(void *arg)
{
    (void) arg;
    int i;
    for (i = 0; i < niterations; i++)
    {
        spin_add(&counter, 1);
    }

    int j;
    for (j = 0; j < niterations; j++)
    {
        spin_add(&counter, -1);
    }
}

void fwrapper4(void *arg)
{
    (void) arg;
    int i;
    for (i = 0; i < niterations; i++)
    {
        swap_add(&counter, 1);
    }

    int j;
    for (j = 0; j < niterations; j++)
    {
        swap_add(&counter, -1);
    }
}

struct option long_options[] =
{
    { "threads",    required_argument, 0, 1 },
    { "iterations", required_argument, 0, 2 },
    { "yield",      required_argument, 0, 3 },
    { "sync",       required_argument, 0, 4 },
};

static int get_arg_as_number_or_exit(const char* opt, char* arg)
{
    char* end;
    int num = (int)strtol(arg, &end, 0);

    if (end == arg)
    {
        fprintf(stderr, "Option \'--%s\' failed: not a valid number: %s\n", opt, arg);
        exit(1);
    }

    return num;
}

int main(int argc, char** argv)
{
    nthreads = 1;
    niterations = 1;
    sync = 1;
    spinlock = 0;
    
    if(pthread_mutex_init(&mutexlock, NULL) != 0)
    {
        fprintf(stderr, "Failed to init the mutex lock!\n");
        exit(1);
    }

    long long startTime;
    long long endTime;
    long long totalTime;
    int operations = 0;

    int c;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if (c == -1)
            break;
        switch (c)
        {
            case 1: // Option "threads"
            {
                int result = get_arg_as_number_or_exit(long_options[long_options_offset - c].name, optarg);
                nthreads = result;
                //fprintf(stderr, "Selected nthreads argument successfully.\n");
                break;
            }
            case 2: // Option "iterations"
            {
                int result = get_arg_as_number_or_exit(long_options[long_options_offset - c].name, optarg);
                niterations = result;
                //fprintf(stderr, "Selected niterations argument successfully.\n");
                break;
            }
            case 3: // Option "yield"
            {
                int result = get_arg_as_number_or_exit(long_options[long_options_offset - c].name, optarg);
                if (result == 1)
                    opt_yield = 1;
                else if (result != 1)
                {
                    fprintf(stderr, "Yield should only be set to 1, if used at all.\n");
                    exit(1);
                }
                break;
            }
            case 4: // Option "sync"
            {
                char* result;
                if ((strchr(optarg, 'm') && strchr(optarg, 's')) || (strchr(optarg, 'c') && strchr(optarg, 's')) || (strchr(optarg, 'm') && strchr(optarg, 'c')))
                {
                    fprintf(stderr, "Option \'--sync\' failed: argument must contain exactly one of [msc]\n");
                    exit(1);
                }
                else if ((result = strchr(optarg, 'm')) || (result = strchr(optarg, 's')) || (result = strchr(optarg, 'c')))
                {
                    switch(*result)
                    {
                        case('m'):
                            sync = 2; 
                            break;
                        case('s'):
                            sync = 3;
                            break;
                        case('c'):
                            sync = 4;
                            break;
                    }
                }
                else
                {
                    fprintf(stderr, "Option \'--sync\' failed: argument does not contain one of [msc]\n");
                    exit(1);
                }

                break;
            }
            default:
                break;
        }
    }

    struct timespec time_start;
    clock_gettime(CLOCK_MONOTONIC, &time_start); 

    int k;
    pthread_t * pth = malloc(nthreads * sizeof(pthread_t));

    switch(sync)
    {
            case 1:    
                for(k = 0; k < nthreads; k++)
                {
                    pthread_create(&pth[k], NULL, (void*) fwrapper1, NULL);
                }
                break;

            case 2:
                for(k = 0; k < nthreads; k++)
                {
                    pthread_create(&pth[k], NULL, (void*) fwrapper2, NULL);
                }
                break;

            case 3:
                for(k = 0; k < nthreads; k++)
                {
                    pthread_create(&pth[k], NULL, (void*) fwrapper3, NULL);
                }
                break;

            case 4:
                for(k = 0; k < nthreads; k++)
                {
                    pthread_create(&pth[k], NULL, (void*) fwrapper4, NULL);
                }
                break;
    }

    for(k = 0; k < nthreads; k++)
    {
        pthread_join(pth[k], NULL);
    }
      
    struct timespec time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);     
    
    startTime = (long long)(time_start.tv_sec*pow(10, 9) + time_start.tv_nsec);
	endTime = (long long)(time_end.tv_sec*pow(10, 9) + time_end.tv_nsec);
	totalTime = endTime - startTime;
    operations = 2 * nthreads * niterations;

    printf("%d threads x %d iterations x (add + subtract) = %d operations\n", nthreads, niterations, operations);

    if(counter != 0)
    {
        fprintf(stderr, "The final counter value is inconsistent! It's value is: %lld \n", counter);
        printf("elapsed time: %lld ns\n", totalTime);
	    printf("per operation: %lld ns\n", totalTime/operations);
        free(pth);
        exit(1);
    }

    free(pth);
    printf("elapsed time: %lld ns\n", totalTime);
	printf("per operation: %lld ns\n", totalTime/operations);
    exit(0);
}
