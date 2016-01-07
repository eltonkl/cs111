#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokens.h"

#define TOKEN_TUPLE(x, y) { y, x##_, "--##x" }
#define FILE_FLAG_OPTION(x) TOKEN_TUPLE(x, file_flag_)
#define FILE_OPEN_OPTION(x) TOKEN_TUPLE(x, file_open_)
#define SUBCOMMAND_OPTION(x) TOKEN_TUPLE(x, subcommand_)
#define MISC_OPTION(x) TOKEN_TUPLE(x, misc_)

option_tuple_t OPTIONS[NUM_OPTIONS] =
{
    FILE_FLAG_OPTION(append),
    FILE_FLAG_OPTION(cloexec),
    FILE_FLAG_OPTION(creat),
    FILE_FLAG_OPTION(directory),
    FILE_FLAG_OPTION(dsync),
    FILE_FLAG_OPTION(excl),
    FILE_FLAG_OPTION(nofollow),
    FILE_FLAG_OPTION(nonblock),
    FILE_FLAG_OPTION(rsync),
    FILE_FLAG_OPTION(sync),
    FILE_FLAG_OPTION(trunc),

    FILE_OPEN_OPTION(rdonly),
    FILE_OPEN_OPTION(rdwr),
    FILE_OPEN_OPTION(wronly),
    FILE_OPEN_OPTION(pipe),

    SUBCOMMAND_OPTION(command),
    SUBCOMMAND_OPTION(wait),

    MISC_OPTION(verbose),
    MISC_OPTION(profile),
    MISC_OPTION(abort),
    MISC_OPTION(catch),
    MISC_OPTION(ignore),
    MISC_OPTION(default),
    MISC_OPTION(pause)
};

bool validate_option(option_t* opt)
{
    //If none of the options are chosen then we don't have a valid option
    if (opt->type < 0 || opt->type > 4)
        return false;
    
    //Otherwise, we must check to make sure that the option has the appropriate number of arguments
    if(opt->type == file_flag_ && opt->num_args != 0)
        return false;
    //All flags have no args
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

//Input: num_tokens > 0 
//Input: args[0] contains an option 
option_t* create_flag(int num_tokens, char** args) 
{ 
    option_t* opt = malloc(sizeof(option_t)); 
    if (!opt) 
        return NULL; 
    for (int i = 0; i < NUM_OPTIONS; i++) 
    { 
        if (strcmp(args[0], OPTIONS[i].value) == 0)
        {
            opt->type = OPTIONS[i].type;
            opt->option = OPTIONS[i].option;
            opt->num_args = num_tokens - 1;
            opt->args = &args[1];
            return opt;
        }
    }
    //This should never occur
    opt->type = unexpected_;
    return opt;
} 
