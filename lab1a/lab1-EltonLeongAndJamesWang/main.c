#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <setjmp.h>
#include <signal.h>

#include "simpsh.h"
#include "options.h"
#include "handlers.h"

int main(int argc, char** argv)
{
    simpsh_init(argc, argv);
    int c;
    while (1)
    {
        if (setjmp(simpsh_context))
        {
            sigset_t ss;
            sigemptyset(&ss);
            sigaddset(&ss, simpsh_last_signal);
            sigprocmask(SIG_UNBLOCK, &ss, NULL);
        }

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
        if (c == ACTIONABLE_OPTION_BASE + 4)
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
        else if (c == ACTIONABLE_OPTION_BASE + 5)
        {
            //This branch occurs if the wait option was called like so: --wait=5
            if (optarg)
                opt = create_actionable_option(c, 1, &argv[optind - 1]);
            else
            {
                if (optind == argc || (strlen(argv[optind]) > 2 && argv[optind][0] == '-' && argv[optind][1] == '-'))
                    opt = create_actionable_option(c, 0, 0);
                else
                    opt = create_actionable_option(c, 1, &argv[optind++]);
            }
        }
        else if (optarg)
            opt = create_actionable_option(c, 1, &argv[optind - 1]);
        else
            opt = create_actionable_option(c, 0, 0);
        execute_actionable_option(opt);
    }
    if (simpsh_profile_perf)
    {
        struct rusage total_self_usage;
        struct rusage total_child_usage;
        getrusage(RUSAGE_SELF, &total_self_usage);
        getrusage(RUSAGE_CHILDREN, &total_child_usage);
        printf("Total parent resource usage\n");
        simpsh_print_rusage(&simpsh_rusage_parent_last);
        printf("Total child resource usage\n");
        simpsh_print_rusage(&simpsh_rusage_child_last);
    }
    simpsh_finish();
    return simpsh_max_status;
}
