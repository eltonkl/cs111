#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

typedef struct token_tuple_t
{
    int type;
    int option;
    char* value;
} token_tuple_t;

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

typedef enum file_open_options_t
{
    rdonly_ = 0,
    rdwr_,
    wronly_,
    pipe_
} file_open_options_t;

typedef enum subcommand_options_t
{
    command_ = 0,
    wait_
} subcommands_options_t;

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

#define TOKEN_TUPLE(x, y) { y, x##_, "--##x" }
#define FILE_FLAG_OPTION(x) TOKEN_TUPLE(x, file_flag_)
#define FILE_OPEN_OPTION(x) TOKEN_TUPLE(x, file_open_)
#define SUBCOMMAND_OPTION(x) TOKEN_TUPLE(x, subcommand_)
#define MISC_OPTION(x) TOKEN_TUPLE(x, misc_)

token_tuple_t TOKENS[] = 
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

typedef struct option_t
{
    token_t type;
    int option;
    int num_args;
    char** args;
} option_t;

bool is_valid_option(option_t* opt);

#endif
