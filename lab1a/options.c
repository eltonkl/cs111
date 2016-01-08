#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "options.h"

#define OPTION_TUPLE(x, y, z) { y, x##_, z }
#define FILE_OPEN_OPTION(x, y) OPTION_TUPLE(x, file_open_, y)
#define SUBCOMMAND_OPTION(x, y) OPTION_TUPLE(x, subcommand_, y)
#define MISC_OPTION(x, y) OPTION_TUPLE(x, misc_, y)

struct option long_options[] =
{
    //flags
    { "append",     no_argument,    &append_flag,       1 },
    { "cloexec",    no_argument,    &cloexec_flag,      1 },
    { "creat",      no_argument,    &creat_flag,        1 },
    { "directory",  no_argument,    &directory_flag,    1 },
    { "dsync",      no_argument,    &dsync_flag,        1 },
    { "excl",       no_argument,    &excl_flag,         1 },
    { "nofollow",   no_argument,    &nofollow_flag,     1 },
    { "nonblock",   no_argument,    &nonblock_flag,     1 },
    { "rsync",      no_argument,    &rsync_flag,        1 },
    { "sync",       no_argument,    &sync_flag,         1 },
    { "trunc",      no_argument,    &trunc_flag,        1 },
    //actionable options
    { "rdonly", required_argument,  0,  'a' },
    { "rdwr",   required_argument,  0,  'b' },
    { "wronly", required_argument,  0,  'c' },
    { "pipe",   no_argument,        0,  'd' },

    { "command",    required_argument,  0,  'e' },
    { "wait",       no_argument,        0,  'f' },

    { "verbose",    no_argument,        0,  'g' },
    { "profile",    no_argument,        0,  'h' },
    { "abort",      no_argument,        0,  'i' },
    { "catch",      required_argument,  0,  'j' },
    { "ignore",     required_argument,  0,  'k' },
    { "default",    required_argument,  0,  'l' },
    { "pause",      no_argument,        0,  'm' }
};

option_tuple_t ACTIONABLE_OPTIONS[] =
{
    FILE_OPEN_OPTION(rdonly, 'a'),
    FILE_OPEN_OPTION(rdwr, 'b'),
    FILE_OPEN_OPTION(wronly, 'c'),
    FILE_OPEN_OPTION(pipe, 'd'),

    SUBCOMMAND_OPTION(command, 'e'),
    SUBCOMMAND_OPTION(wait, 'f'),

    MISC_OPTION(verbose, 'g'),
    MISC_OPTION(profile, 'h'),
    MISC_OPTION(abort, 'i'),
    MISC_OPTION(catch, 'j'),
    MISC_OPTION(ignore, 'k'),
    MISC_OPTION(default, 'l'),
    MISC_OPTION(pause, 'm')
};

void zero_flags()
{
    append_flag = 0;
    cloexec_flag = 0;
    creat_flag = 0;
    directory_flag = 0;
    dsync_flag = 0;
    excl_flag = 0;
    nofollow_flag = 0;
    nonblock_flag = 0;
    rsync_flag = 0;
    sync_flag = 0;
    trunc_flag = 0;
}

bool check_valid_option(char** argv, const option_t* opt)
{
    //If none of the options are chosen then we don't have a valid option
    if (opt->type < file_open_ || opt->type > unexpected_)
        return false;
    int error_offset = 0; 
    //Otherwise, we must check to make sure that the option has the appropriate number of arguments
    if(opt->type == file_open_)
    //All opens have one arg, except pipe which has no args
    {
        if((opt->option == rdwr_ || opt->option == rdonly_ || opt->option == wronly_) && opt->num_args != 1)
        {
            error_offset = OFFSET_FILE_OPEN_OPTIONS;
            goto fail;
        }
        else if(opt->option == pipe_ && opt->num_args != 0)
        {
            error_offset = OFFSET_FILE_OPEN_OPTIONS;
            goto fail;
        }
    }
    else if(opt->type == subcommand_)
    //command has i e o cmd and 0 or more args, wait has no args
    {
        if(opt->option == command_ && opt->num_args < 4) 
        {
            error_offset = OFFSET_SUBCOMMAND_OPTIONS;
            goto fail;
        }
        else if(opt->option == wait_ && opt->num_args != 0)
        {
            error_offset = OFFSET_SUBCOMMAND_OPTIONS;
            goto fail;
	}
    }
    else if(opt->type == misc_)
    //Some misc have no args, others have one arg
    {
        if ((opt->option == catch_ || opt->option == ignore_ || opt->option == default_) && opt->num_args != 1)
        {
            error_offset = OFFSET_MISC_OPTIONS;
            goto fail;
        }
        else if ((opt->option == verbose_ || opt->option == profile_ || opt->option == abort_ || opt->option == pause_) && opt->num_args != 0)
        {
            error_offset = OFFSET_MISC_OPTIONS;
            goto fail;
        }   
    }
	
    //If we make it this far, then our option is indeed valid.
    return true;
fail:
    fprintf(stderr, "%s: invalid number of args for option \'--%s\'\n",
            argv[0], long_options[error_offset + opt->option].name);
    return false;
}

option_t create_actionable_option(char val, int num_args, char** args) 
{ 
    option_t opt; 
    for (int i = 0; i < NUM_OPTIONS; i++) 
    { 
        if (val == ACTIONABLE_OPTIONS[i].val)
        {
            opt.type = ACTIONABLE_OPTIONS[i].type;
            opt.option = ACTIONABLE_OPTIONS[i].option;
            opt.num_args = num_args;
            if (num_args > 0)
                opt.args = args;
            else
                opt.args = 0;
            return opt;
        }
    }
    //This should never occur
    opt.type = unexpected_;
    return opt;
}
