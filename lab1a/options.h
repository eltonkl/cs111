#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

typedef struct option_tuple_t
{
    int type;
    int option;
    char* value;
} option_tuple_t;

typedef enum option_type_t
{
    file_flag_ = 0,
    file_open_,
    subcommand_,
    misc_,
    unexpected_
} option_type_t;

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

#define NUM_OPTIONS 24
extern option_tuple_t OPTIONS[NUM_OPTIONS];

typedef struct option_t
{
    option_type_t type;
    int option;
    int num_args;
    char** args;
} option_t;

bool is_valid_option(option_t* opt);

#endif
