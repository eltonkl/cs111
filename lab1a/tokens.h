#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

typedef enum token 
{
    file_flag,
    file_open,
    subcommand,
    misc,
    unexpected
} token;

typedef enum file_flag_options
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
} file_flags;

typedef enum file_open_options
{
    rdonly,
    rdwr,
    wronly,
    pipe
} file_open_options;

typedef enum subcommand_options
{
    command,
    wait
} subcommands;

typedef enum misc_options
{
    verbose,
    profile,
    abort_,
    catch_,
    ignore,
    default_,
    pause
} misc_options;

typedef struct flag
{
    token type;
    void* option;
    int num_args;
    char** args;
} flag;

bool validate_flag(flag* f);

#endif
