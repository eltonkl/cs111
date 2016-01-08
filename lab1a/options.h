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
#define NUM_FILE_OPEN_OPTIONS 4
#define NUM_SUBCOMMAND_OPTIONS 2
#define NUM_MISC_OPTIONS 7

#define OFFSET_FILE_OPEN_OPTIONS NUM_FLAGS
#define OFFSET_SUBCOMMAND_OPTIONS (OFFSET_FILE_OPEN_OPTIONS + NUM_FILE_OPEN_OPTIONS)
#define OFFSET_MISC_OPTIONS (OFFSET_SUBCOMMAND_OPTIONS + NUM_SUBCOMMAND_OPTIONS)

extern struct option long_options[NUM_OPTIONS];

typedef struct option_tuple_t
{
    int type;
    int option;
    char val;
} option_tuple_t;

#define NUM_ACTIONABLE_OPTIONS (NUM_OPTIONS - NUM_FLAGS)
extern option_tuple_t ACTIONABLE_OPTIONS[NUM_ACTIONABLE_OPTIONS];

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
bool check_valid_option(char** argv, const option_t* opt);
option_t create_actionable_option(char val, int num_args, char** args);

#endif
