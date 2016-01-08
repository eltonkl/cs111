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
        else if(c == 0)
        {
            //if (long_options[option_index].flag != 0)
            //Flag was set
        }
        else if (c == '?')
        {
            //getopt_long already printed error msg
        }
        else
        {
            option_t opt;
            //pass args to create_option and then check
            if (optarg)
            {
                int num_args = 1;
                if (optind < argc)
                {
                    int index = optind;
                    while (index < argc)
                    {
                        if (strncmp("--", argv[index++], 2) != 0)
                            num_args++;
                        else
                            break;
                    }
                }
                opt = create_actionable_option(long_options[option_index].val,
                                               num_args, &argv[optind - 1]);
            }
            else
                opt = create_actionable_option(long_options[option_index].val, 0, 0);
            bool valid = check_valid_option(argv, &opt);
        }
    }
    return 0;
}
