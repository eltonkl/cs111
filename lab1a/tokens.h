#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

typedef enum token_t
{
    file_flag,
    file_open,
    subcommand,
    misc,
    unexpected
} token_t;

typedef enum file_flag_options_t
{
    append,
    cloexec,
    creat,
    directory,
    dsync,
    excl,
    nofollow,
    nonblock,
    rsync,
    sync,
    trunc
} file_flag_option_t;

typedef enum file_open_options_t
{
    rdonly,
    rdwr,
    wronly,
    pipe
} file_open_options_t;

typedef enum subcommand_options_t
{
    command,
    wait
} subcommands_options_t;

typedef enum misc_options_t
{
    verbose,
    profile,
    abort_,
    catch_,
    ignore,
    default_,
    pause
} misc_options_t;

typedef struct option_t
{
    token_t type;
    int option;
    int num_args;
    char** args;
} option_t;

bool validate_option(option_t* f);

#endif
