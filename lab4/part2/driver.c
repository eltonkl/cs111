#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
int num_threads = 0;
int num_iterations = 0;
int num_lists = 0;
bool check_inserts = false;
bool check_deletes = false;
bool check_lookups = false;

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
                lists = get_arg_as_number_or_exit(long_options[c - long_options_offset].name, optarg);
                break;
            }
            default:
                break;
        }
    }
}
