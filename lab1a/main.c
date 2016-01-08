#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "error.h"
#include "options.h"

int append_flag;
int cloexec_flag;
int creat_flag;
int directory_flag;
int dsync_flag;
int excl_flag;
int nofollow_flag;
int nonblock_flag;
int rsync_flag;
int sync_flag;
int trunc_flag;

bool simpsh_verbose;

int main(int argc, char** argv)
{
    zero_flags();
    int c;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if (c == -1)
            break;
        else if (c == 0)
        {
            //if (long_options[option_index].flag != 0)
            //Flag was set
            continue;
        }
        else if (c == '?')
        {
            //getopt_long already printed error msg
            continue;
        }

        //pass args to create_option and then check
        option_t opt;
        if (c == 'e')
        { 
            if (!optarg)
            {
                fprintf(stderr, "%s: invalid number of args for option \'--command\'\n", argv[0]); 
                continue;
            }
            else
            {
                int num_args = 1;
                int original_optind = optind;
                if (optind < argc)
                {
                    while (optind < argc)
                    {
                        if (strncmp("--", argv[optind], 2) != 0)
                        {
                            num_args++;
                            optind++;
                        }
                        else
                            break;
                    }
                }
                opt = create_actionable_option(long_options[option_index].val,
                                               num_args, &argv[original_optind - 1]);
            }
        }
        else if (optarg)
            opt = create_actionable_option(long_options[option_index].val, 1, &argv[optind - 1]);
        else
            opt = create_actionable_option(long_options[option_index].val, 0, 0);
        bool valid = is_valid_option(argv, &opt);
        if (!valid)
            continue;
    }
    return 0;
}
