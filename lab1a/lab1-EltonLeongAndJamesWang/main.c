#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>

#include "simpsh.h"
#include "options.h"
#include "handlers.h"

int main(int argc, char** argv)
{
    simpsh_init(argc, argv);
    int c;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if (c == -1)
            break;
        else if (c == 0)
        {
            //Flag was set
            if (simpsh_print_verbose)
            {
                printf("--%s\n", long_options[option_index].name);
                fflush(stdout);
            }
            continue;
        }
        else if (c == '?')
        {
            simpsh_error_set_status();
            continue;
        }

        //pass args to create_actionable_option
        option_t opt;
        if (c == 'e')
        { 
            if (!optarg)
            {
                fprintf(stderr, "%s: invalid number of args for option \'--command\'\n", argv[0]);
                simpsh_error_set_status();
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
                        //Consume everything until the next long option
                        if (strlen(argv[optind]) > 2 && argv[optind][0] == '-' && argv[optind][1] == '-')
                            break;
                        num_args++;
                        optind++;
                    }
                }
                opt = create_actionable_option(c, num_args,
                                               &argv[original_optind - 1]);
                if (!is_valid_command(argv, opt))
                {
                    simpsh_error_set_status();
                    continue;
                }
            }
        }
        else if (optarg)
            opt = create_actionable_option(c, 1, &argv[optind - 1]);
        else
            opt = create_actionable_option(c, 0, 0);
        execute_actionable_option(opt);
    }
    simpsh_finish();
    return simpsh_max_status;
}