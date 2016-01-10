#ifndef VECTOR_H
#define VECTOR_H

#include "options.h"

typedef struct command_vector
{
    int* pid_data;
    struct option_t* command_data; 
    int cur;
    int max;
} *command_vector_t;

command_vector_t simpsh_command_vector_create();
bool simpsh_command_vector_get_data(command_vector_t vec, int index, int* pid_storage, option_t* command_storage);
bool simpsh_command_vector_insert(command_vector_t vec, int pid, option_t command);
int simpsh_command_vector_get_count(command_vector_t vec);
void simpsh_command_vector_remove_by_index(command_vector_t vec, int index);
void simpsh_command_vector_delete(command_vector_t vec);

#endif
