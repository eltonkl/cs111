#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "SortedList.h"

const int long_options_offset = 1;

struct option long_options[] =
{
    { "threads",    required_argument, 0, 1 },
    { "iterations", required_argument, 0, 2 },
    { "yield",      required_argument, 0, 3 },
    { "sync",       required_argument, 0, 4 },
    { "lists",      required_argument, 0, 5 },
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

enum { sync_none, sync_mutex, sync_spinlock } sync_type = sync_none;
int num_threads = 1;
int num_iterations = 1;
int num_lists = 0;
bool check_inserts = false;
bool check_deletes = false;
bool check_lookups = false;
int opt_yield = 0;
SortedList_t sl;
bool error = false;

void* process(void*);

int main(int argc, char** argv)
{
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
                num_threads = get_arg_as_number_or_exit(long_options[c - long_options_offset].name, optarg);
                break;
            }
            case 2: // Option "iterations"
            {
                num_iterations = get_arg_as_number_or_exit(long_options[c - long_options_offset].name, optarg);
                break;
            }
            case 3: // Option "yield"
            {
                if (!strchr(optarg, 'i') && !strchr(optarg, 'd') && !strchr(optarg, 's'))
                {
                    fprintf(stderr, "Option \'--yield\' failed: argument does not contain at least one of \'ids\'\n");
                    exit(1);
                }
                if (strchr(optarg, 'i'))
                    check_inserts = true;
                if (strchr(optarg, 'd'))
                    check_deletes = true;
                if (strchr(optarg, 's'))
                    check_lookups = true;
                break;
            }
            case 4: // Option "sync"
            {
                char* result;
                if (strchr(optarg, 'm') && strchr(optarg, 's'))
                {
                    fprintf(stderr, "Option \'--sync\' failed: argument must contain one of \'ms\', not both\n");
                    exit(1);
                }
                else if ((result = strchr(optarg, 'm')) || (result = strchr(optarg, 's')))
                {
                    if (*result == 'm')
                        sync_type = sync_mutex;
                    else
                        sync_type = sync_spinlock;
                }
                else
                {
                    fprintf(stderr, "Option \'--sync\' failed: argument does not contain one of \'ms\'\n");
                    exit(1);
                }

                break;
            }
            case 5: // Option "lists"
            {
                num_lists = get_arg_as_number_or_exit(long_options[c - long_options_offset].name, optarg);
                break;
            }
            default:
                break;
        }
    }

    if (num_threads <= 0)
    {
        fprintf(stderr, "Number of threads must be > 0\n");
        exit(1);
    }
    if (num_iterations <= 0)
    {
        fprintf(stderr, "Number of iterations must be > 0\n");
        exit(1);
    }
    if (num_lists < 0)
    {
        fprintf(stderr, "Number of lists must be nonnegative\n");
        exit(1);
    }

    if (check_deletes)
        opt_yield |= DELETE_YIELD;
    if (check_inserts)
        opt_yield |= INSERT_YIELD;
    if (check_lookups)
        opt_yield |= SEARCH_YIELD;

    sl.next = &sl;
    sl.prev = &sl;
    sl.key = NULL;

    SortedListElement_t* elements = (SortedListElement_t*)malloc(sizeof(SortedListElement_t) * num_threads * num_iterations);
    if (!elements)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    srand(time(NULL));
    for (int i = 0; i < num_threads * num_iterations; i++)
    {
        int x = rand();
        x %= 99999999;
        char* result = (char*)malloc(sizeof(char) * 9); 
        if (!result)
        {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        result[0] = '\0';
        if (snprintf(result, 9, "%i", x) < 0)
        {
            fprintf(stderr, "int to c-string conversion failed\n");
            exit(1);
        }
        elements[i].key = result;
    }

    /*for (int i = 0; i < num_threads * num_iterations; i++)
        SortedList_insert(&sl, &elements[i]);
    printf("%d length\n", SortedList_length(&sl));*/

    struct timespec start_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }

    pthread_t* tids = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (tids == NULL)
    {
        fprintf(stderr, "error allocating memory");
        exit(1);
    }

    for (int i = 0; i < num_threads; i++)
    {
        int result = pthread_create(&tids[i], NULL, process, (void *)&elements[i * num_iterations]);
        if (result == 1)
        {
            fprintf(stderr, "error creating thread");
            exit(1);
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        void* result;
        if (pthread_join(tids[i], &result) == 1)
        {
            fprintf(stderr, "error joining thread");
            return 1;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }

    if (SortedList_length(&sl) != 0)
    {
        fprintf(stderr, "list length is not 0\n");
        error = true;
    }

    for (int i = 0; i < num_threads * num_iterations; i++)
        free((void*)elements[i].key);
    free(elements);

    long long elapsed = 1000000000L * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_nsec - start_time.tv_nsec;
    printf("%d threads x %d iterations x (ins + lookup/del) x (8 avg len) = %d operations\n", num_threads, num_iterations, num_threads * num_iterations * 8);
    printf("elapsed time: %lld ns\nper operation: %lld ns\n", elapsed, elapsed/(num_threads * num_iterations * 8));

    if (error)
        exit(1);
    return 0;
}

void* process(void* elements)
{
    for (int i = 0; i < num_iterations; i++)
        SortedList_insert(&sl, &((SortedListElement_t*)elements)[i]);
    SortedList_length(&sl);
    for (int i = 0; i < num_iterations; i++)
        SortedList_delete(SortedList_lookup(&sl, ((SortedListElement_t*)elements)[i].key));
    pthread_exit(NULL);
}
