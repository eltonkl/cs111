#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <getopt.h>

extern int append_flag;
extern int cloexec_flag;
extern int creat_flag;
extern int directory_flag;
extern int dsync_flag;
extern int excl_flag;
extern int nofollow_flag;
extern int nonblock_flag;
extern int rsync_flag;
extern int sync_flag;
extern int trunc_flag;

#define NUM_OPTIONS 24
#define NUM_FLAGS 11
static struct option long_options[] =
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

typedef struct option_tuple_t
{
    int type;
    int option;
    char val;
} option_tuple_t;

extern option_tuple_t OPTIONS[NUM_OPTIONS - NUM_FLAGS];

typedef enum option_type_t
{
    file_open_ = 0,
    subcommand_,
    misc_,
    unexpected_
} option_type_t;

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

typedef struct option_t
{
    option_type_t type;
    int option;
    int num_args;
    char** args;
} option_t;

void zero_flags();
bool check_valid_option(option_t* opt);
option_t* create_option(char val, int num_tokens, char** tokens);

#endif
