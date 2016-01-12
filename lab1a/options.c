#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "options.h"
#include "functions.h"
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
    { "wait",       no_argument,        0,  ACTIONABLE_OPTION_BASE + 5  },

    { "verbose",    no_argument,        0,  ACTIONABLE_OPTION_BASE + 6  },
    { "profile",    no_argument,        0,  ACTIONABLE_OPTION_BASE + 7  },
    { "abort",      no_argument,        0,  ACTIONABLE_OPTION_BASE + 8  },
    { "catch",      required_argument,  0,  ACTIONABLE_OPTION_BASE + 9  },
    { "ignore",     required_argument,  0,  ACTIONABLE_OPTION_BASE + 10 },
    { "default",    required_argument,  0,  ACTIONABLE_OPTION_BASE + 11 },
    { "pause",      no_argument,        0,  ACTIONABLE_OPTION_BASE + 12 },
    { 0, 0, 0, 0 }
};

bool is_valid_command(char** argv, option_t opt)
{
    if (opt.num_args < 4)
    {
        fprintf(stderr, "%s: invalid number of args for option \'--command\'\n", argv[0]);
        return false;
    }
    for (int i = 0; i < 3; i++)
    {
        char* end;
        fd_t fd;
        int logical_fd_num = (int)strtol(opt.args[i], &end, 0);
        if (end == opt.args[i])
        {
            fprintf(stderr, "Option \'--command\' failed: not a valid number: %s\n", opt.args[i]);
            return false;
        }
        if (!simpsh_get_fd(logical_fd_num, &fd))
        {
            fprintf(stderr, "Option \'--command\' failed: not a valid file or pipe number: %s\n", opt.args[i]);
            return false;
        }
        if (fd.type == SIMPSH_CLOSED)
        {
            fprintf(stderr, "Option \'--command\' failed: pipe already consumed\n");
            return false;
        }
    }
    return true;
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
    opt.function = simpsh_functions[val - ACTIONABLE_OPTION_BASE];
    return opt;
}

void execute_actionable_option(option_t opt)
{
    if (simpsh_print_verbose)
        print_actionable_option_with_args(opt);
    opt.function(opt);
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
