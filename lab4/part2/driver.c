#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
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

typedef struct list
{
    SortedList_t sl;
    pthread_mutex_t mutex;
    int lock;
} list_t;


enum { sync_none, sync_mutex, sync_spinlock } sync_type = sync_none;
int num_threads = 1;
int num_iterations = 1;
int num_lists = 1;
int opt_yield = 0;
list_t* lists;
int* spinlocks;
pthread_mutex_t* mutexes;
SortedListElement_t* elements = NULL;

static int get_arg_as_number_or_exit(const char* opt, char* arg);
void* process(void*);
int get_length_of_all_lists();
int get_hash(const char* key);
void acquire_lock(int i);
void release_lock(int i);

int main(int argc, char** argv)
{
    bool error = false;
    bool check_inserts = false;
    bool check_deletes = false;
    bool check_lookups = false;
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
    if (num_lists <= 0)
    {
        fprintf(stderr, "Number of lists must be > 0\n");
        exit(1);
    }

    if (check_deletes)
        opt_yield |= DELETE_YIELD;
    if (check_inserts)
        opt_yield |= INSERT_YIELD;
    if (check_lookups)
        opt_yield |= SEARCH_YIELD;

    lists = (list_t*)malloc(sizeof(list_t) * num_lists);
    spinlocks = (int*)malloc(sizeof(int) * num_lists);
    mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_lists);
    if (!lists || !spinlocks || !mutexes)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    for (int i = 0; i < num_lists; i++)
    {
        lists[i].sl.next = &lists[i].sl;
        lists[i].sl.prev = &lists[i].sl;
        lists[i].sl.key = NULL;

        if (sync_type == sync_spinlock)
            spinlocks[i] = 0;
        else if (sync_type == sync_mutex)
            pthread_mutex_init(&mutexes[i], NULL);
    }

    elements = (SortedListElement_t*)malloc(sizeof(SortedListElement_t) * num_threads * num_iterations);
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

    //for (int i = 0; i < num_threads * num_iterations; i++)
        //SortedList_insert(&lists[0].sl, &elements[i]);
    //printf("%d length\n", SortedList_length(&lists[0].sl));

    pthread_t* threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL)
    {
        fprintf(stderr, "error allocating memory\n");
        exit(1);
    }

    struct timespec start_time, end_time;
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }

    for (int i = 0; i < num_threads; i++)
    {
        int result = pthread_create(&threads[i], NULL, process, (void*)(uintptr_t)i);
        if (result == 1)
        {
            fprintf(stderr, "error creating thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        void* result;
        if (pthread_join(threads[i], &result) == 1)
        {
            fprintf(stderr, "error joining thread\n");
            return 1;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }

    long long elapsed = 1000000000L * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_nsec - start_time.tv_nsec;
    printf("%d threads x %d iterations x (ins + lookup/del) x (8 avg len) = %d operations\n", num_threads, num_iterations, num_threads * num_iterations * 3 * 8);
    printf("elapsed time: %lld ns\nper operation: %lld ns\n", elapsed, elapsed/(num_threads * num_iterations * 3 * 8));

    if (get_length_of_all_lists() != 0)
    {
        fprintf(stderr, "list length is not 0\n");
        error = true;
    }

    for (int i = 0; i < num_lists; i++)
        if (sync_type == sync_mutex)
            pthread_mutex_destroy(&mutexes[i]);
    for (int i = 0; i < num_threads * num_iterations; i++)
        free((void*)elements[i].key);
    free(elements);
    free(threads);
    free(spinlocks);
    free(mutexes);
    free(lists);

    if (error)
        exit(1);
    return 0;
}

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

void* process(void* thread_num)
{
    SortedListElement_t* start = &elements[*((int*)&thread_num) * num_iterations];

    for (int i = 0; i < num_iterations; i++)
    {
        int hash = get_hash(start[i].key);
        acquire_lock(hash);
        SortedList_insert(&lists[hash].sl, &start[i]);
        release_lock(hash);
    }
    get_length_of_all_lists();
    for (int i = 0; i < num_iterations; i++)
    {
        int hash = get_hash(start[i].key);
        acquire_lock(hash);
        SortedList_delete(SortedList_lookup(&lists[hash].sl, start[i].key));
        release_lock(hash);
    }
    pthread_exit(NULL);
}

int get_length_of_all_lists()
{
    int total = 0;
    for (int i = 0; i < num_lists; i++)
    {
        acquire_lock(i);
        total += SortedList_length(&lists[i].sl);
        release_lock(i);
    }
    return total;
}

int get_hash(const char* key)
{
    int hash = 0;
    while (*key != '\0')
        hash = (hash + (hash >> 5)) + *(key++);
    return hash % num_lists;
}

void acquire_lock(int i)
{
    if (sync_type == sync_spinlock)
    {
        while (__sync_lock_test_and_set(&spinlocks[i], 1))
            continue;
    }
    else if (sync_type == sync_mutex)
        pthread_mutex_lock(&mutexes[i]);
}

void release_lock(int i)
{
    if (sync_type == sync_spinlock)
        __sync_lock_release(&spinlocks[i]);
    else if (sync_type == sync_mutex)
        pthread_mutex_unlock(&mutexes[i]);
}
