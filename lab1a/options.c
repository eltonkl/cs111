#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "options.h"

#define OPTION_TUPLE(x, y, z) { y, x##_, z }
#define FILE_OPEN_OPTION(x, y) OPTION_TUPLE(x, file_open_, y)
#define SUBCOMMAND_OPTION(x, y) OPTION_TUPLE(x, subcommand_, y)
#define MISC_OPTION(x, y) OPTION_TUPLE(x, misc_, y)

option_tuple_t OPTIONS[] =
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

bool check_valid_option(option_t* opt)
{
    //If none of the options are chosen then we don't have a valid option
    if (opt->type < file_open_ || opt->type > unexpected_)
        return false;
    
    //Otherwise, we must check to make sure that the option has the appropriate number of arguments
    if(opt->type == file_open_)
    //All opens have one arg, except pipe which has no args
    {
        if((opt->option == rdwr_ || opt->option == rdonly_ || opt->option == wronly_) && opt->num_args != 1) 
            return false;
        if(opt->option == pipe_ && opt->num_args != 0)
            return false;
    }
	
    if(opt->type == subcommand_)
    //command has i e o cmd and 0 or more args, wait has no args
    {
        if(opt->option == command_ && opt->num_args < 4) 
            return false;
        if(opt->option == wait_ && opt->num_args != 0)
            return false;
	}
	
    if(opt->type == misc_)
    //Some misc have no args, others have one arg
    {
        if ((opt->option == catch_ || opt->option == ignore_ || opt->option == default_) && opt->num_args != 1)
            return false;
        if ((opt->option == verbose_ || opt->option == profile_ || opt->option == abort_ || opt->option == pause_) && opt->num_args != 0)
            return false;
    }
	
    //If we make it this far, then our option is indeed valid.
    return true;
}

//Input: args[0] contains an option
option_t* create_option(char val, int num_tokens, char** tokens) 
{ 
    option_t* opt = malloc(sizeof(option_t)); 
    if (!opt) 
        return NULL; 
    for (int i = 0; i < NUM_OPTIONS; i++) 
    { 
        if (val == OPTIONS[i].val)
        {
            opt->type = OPTIONS[i].type;
            opt->option = OPTIONS[i].option;
            opt->num_args = num_tokens;
            opt->args = tokens;
            return opt;
        }
    }
    //This should never occur
    opt->type = unexpected_;
    return opt;
}
