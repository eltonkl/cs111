#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <getopt.h>

typedef struct option_t
{
    const char* name;
    int num_args;
    char** args;
    void (*function)(struct option_t);
} option_t;

#define NUM_OPTIONS 24
#define NUM_FLAGS 11
#define NUM_FILE_OPEN_OPTIONS 4
#define NUM_SUBCOMMAND_OPTIONS 2
#define NUM_MISC_OPTIONS 7

#define NUM_ACTIONABLE_OPTIONS (NUM_OPTIONS - NUM_FLAGS)
#define ACTIONABLE_OPTION_BASE 'a'

#define OFFSET_ACTIONABLE_OPTIONS NUM_FLAGS
extern struct option long_options[NUM_OPTIONS + 1];

bool is_valid_command(char** argv, option_t opt);
option_t create_actionable_option(int val, int num_args, char** args);
void execute_actionable_option(option_t opt);

#endif
