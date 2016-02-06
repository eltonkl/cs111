#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <getopt.h>

typedef struct option_t
{
    const char* name;   //Name of option
    int num_args;       //Number of arguments
    char** args;        //Pointer to arguments for this function
    void (*handler)(struct option_t);   //Function pointer
} option_t;

#define NUM_OPTIONS 25
#define NUM_FLAGS 11
#define NUM_FILE_OPEN_OPTIONS 4
#define NUM_SUBCOMMAND_OPTIONS 2
#define NUM_MISC_OPTIONS 8

#define NUM_ACTIONABLE_OPTIONS (NUM_OPTIONS - NUM_FLAGS)
#define ACTIONABLE_OPTION_BASE 'a'

#define OFFSET_ACTIONABLE_OPTIONS NUM_FLAGS
extern struct option long_options[NUM_OPTIONS + 1];

bool is_valid_command(char** argv, option_t opt);
option_t create_actionable_option(int val, int num_args, char** args);
void execute_actionable_option(option_t opt);
void print_actionable_option_with_args(option_t opt);

#endif
