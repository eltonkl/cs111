#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

#define TOKEN_PAIR(x) { x##_, "--##x" }

typedef struct token_pair_t
{
    int option;
    char* value;
} token_pair_t;

typedef enum token_t
{
    file_flag_ = 0,
    file_open_,
    subcommand_,
    misc_,
    unexpected_
} token_t;

typedef enum file_flag_options_t
{
    append_ = 0,
    cloexec_,
    creat_,
    directory_,
    dsync_,
    excl_,
    nofollow_,
    nonblock_,
    rsync_,
    sync_,
    trunc_
} file_flag_option_t;

token_pair_t FILE_FLAG_OPTION_TOKENS[] = 
{
    TOKEN_PAIR(append),
    TOKEN_PAIR(cloexec),
    TOKEN_PAIR(creat),
    TOKEN_PAIR(directory),
    TOKEN_PAIR(dsync),
    TOKEN_PAIR(excl),
    TOKEN_PAIR(nofollow),
    TOKEN_PAIR(nonblock),
    TOKEN_PAIR(rsync),
    TOKEN_PAIR(sync),
    TOKEN_PAIR(trunc)
};

typedef enum file_open_options_t
{
    rdonly_ = 0,
    rdwr_,
    wronly_,
    pipe_
} file_open_options_t;

token_pair_t FILE_OPEN_OPTION_TOKENS[] =
{
    TOKEN_PAIR(rdonly),
    TOKEN_PAIR(rdwr),
    TOKEN_PAIR(wronly),
    TOKEN_PAIR(pipe)
};

typedef enum subcommand_options_t
{
    command_ = 0,
    wait_
} subcommands_options_t;

token_pair_t SUBCOMMAND_OPTION_TOKENS[] =
{
    TOKEN_PAIR(command),
    TOKEN_PAIR(wait)
};

typedef enum misc_options_t
{
    verbose_ = 0,
    profile_,
    abort_,
    catch_,
    ignore_,
    default_,
    pause_
} misc_options_t;

token_pair_t MISC_OPTION_TOKENS[] =
{
    TOKEN_PAIR(verbose),
    TOKEN_PAIR(profile),
    TOKEN_PAIR(abort),
    TOKEN_PAIR(catch),
    TOKEN_PAIR(ignore),
    TOKEN_PAIR(default),
    TOKEN_PAIR(pause)
};

typedef struct option_t
{
    token_t type;
    int option;
    int num_args;
    char** args;
} option_t;

bool is_valid_option(option_t* opt);

#endif
