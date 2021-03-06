#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "options.h"
#include "handlers.h"
#include "simpsh.h"

struct option long_options[] =
{
    //flags
    { "append",     no_argument,        &append_flag,       1   },
    { "cloexec",    no_argument,        &cloexec_flag,      1   },
    { "creat",      no_argument,        &creat_flag,        1   },
    { "directory",  no_argument,        &directory_flag,    1   },
    { "dsync",      no_argument,        &dsync_flag,        1   },
    { "excl",       no_argument,        &excl_flag,         1   },
    { "nofollow",   no_argument,        &nofollow_flag,     1   },
    { "nonblock",   no_argument,        &nonblock_flag,     1   },
    { "rsync",      no_argument,        &rsync_flag,        1   },
    { "sync",       no_argument,        &sync_flag,         1   },
    { "trunc",      no_argument,        &trunc_flag,        1   },
    //actionable options
    { "rdonly",     required_argument,  0,  ACTIONABLE_OPTION_BASE + 0  },
    { "rdwr",       required_argument,  0,  ACTIONABLE_OPTION_BASE + 1  },
    { "wronly",     required_argument,  0,  ACTIONABLE_OPTION_BASE + 2  },
    { "pipe",       no_argument,        0,  ACTIONABLE_OPTION_BASE + 3  }, 

    { "command",    required_argument,  0,  ACTIONABLE_OPTION_BASE + 4  },
    { "wait",       optional_argument,  0,  ACTIONABLE_OPTION_BASE + 5  },

    { "verbose",    no_argument,        0,  ACTIONABLE_OPTION_BASE + 6  },
    { "profile",    no_argument,        0,  ACTIONABLE_OPTION_BASE + 7  },
    { "abort",      no_argument,        0,  ACTIONABLE_OPTION_BASE + 8  },
    { "catch",      required_argument,  0,  ACTIONABLE_OPTION_BASE + 9  },
    { "ignore",     required_argument,  0,  ACTIONABLE_OPTION_BASE + 10 },
    { "default",    required_argument,  0,  ACTIONABLE_OPTION_BASE + 11 },
    { "pause",      no_argument,        0,  ACTIONABLE_OPTION_BASE + 12 },
    { "close",      required_argument,  0,  ACTIONABLE_OPTION_BASE + 13 },
    { 0, 0, 0, 0 }
};

bool is_valid_command(char** argv, option_t opt)
{
    if (opt.num_args < 4)
    {
        fprintf(stderr, "%s: invalid number of args for option \'--command\', args ", argv[0]);
        goto fail;
    }
    for (int i = 0; i < 3; i++)
    {
        char* end;
        fd_t fd;
        int logical_fd_num = (int)strtol(opt.args[i], &end, 0);
        if (end == opt.args[i])
        {
            fprintf(stderr, "\'--command\' failed: not a valid number: %s, args ", opt.args[i]);
            goto fail;
        }
        if (!simpsh_get_fd(logical_fd_num, &fd))
        {
            fprintf(stderr, "\'--command\' failed: not a valid file or pipe number: %s, args ", opt.args[i]);
            goto fail;
        }
        if (fd.type == SIMPSH_CLOSED)
        {
            fprintf(stderr, "\'--command\' failed: pipe already consumed, args ");
            goto fail;
        }
    }
    return true;
fail:
    print_actionable_option_with_args(opt);
    return false;
}

option_t create_actionable_option(int val, int num_args, char** args)
{
    option_t opt;
    opt.name = long_options[OFFSET_ACTIONABLE_OPTIONS + 
                            (val - ACTIONABLE_OPTION_BASE)].name;
    opt.num_args = num_args;
    if (num_args > 0)
        opt.args = args;
    else
        opt.args = 0;
    opt.handler = simpsh_handlers[val - ACTIONABLE_OPTION_BASE];
    return opt;
}

void execute_actionable_option(option_t opt)
{
    struct rusage new_rusage;
    //Control whether or not we use verbose
    if (simpsh_print_verbose)
        print_actionable_option_with_args(opt);
    //This function will execute the option
    //The argument is a function pointer to the handler that we want, these can be found in handlers.c/handlers.h
    opt.handler(opt);
    if (simpsh_profile_perf)
    {
        printf("Parent resource usage for ");
        print_actionable_option_with_args(opt);
        getrusage(RUSAGE_SELF, &new_rusage);
        simpsh_print_rusage_diff(&simpsh_rusage_parent_last, &new_rusage);
        simpsh_rusage_parent_last = new_rusage;
    }
}

void print_actionable_option_with_args(option_t opt)
{
    printf("--%s", opt.name);
    if (opt.num_args > 0)
    {
        for (int i = 0; i < opt.num_args; i++)
            printf(" %s", opt.args[i]);
    }
    putchar('\n'); 
    fflush(stdout);
}
