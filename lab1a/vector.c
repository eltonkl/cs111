#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "vector.h"
#include "options.h"

command_vector_t simpsh_command_vector_create()
{
    command_vector_t vec = (command_vector_t)malloc(sizeof(struct command_vector));
    int* pid_storage = (int *)malloc(sizeof(int));
    option_t* command_storage = (option_t*)malloc(sizeof(option_t));
    if (pid_storage == NULL || vec == NULL || command_storage == NULL)
    {
        fprintf(stderr, "Failed to allocate storage\n");
        free(vec);
        free(pid_storage);
        free(command_storage);
        return NULL;
    }
    vec->pid_data = pid_storage;
    vec->command_data = command_storage;
    vec->cur = 0;
    vec->max = 1;
    return vec;
}

bool simpsh_command_vector_get_data(command_vector_t vec, int index, int* pid_storage, option_t* command_storage)
{
    if (vec == NULL)
        return false;
    if (vec->pid_data == NULL || vec->command_data == NULL)
        return false;
    if (index < 0 || index >= vec->cur)
        return false;
    if (pid_storage)
        *pid_storage = vec->pid_data[index];
    if (command_storage)
        *command_storage = vec->command_data[index];
    return true;
}

bool simpsh_command_vector_insert(command_vector_t vec, int pid, option_t command)
{
    if (vec == NULL)
        return false;
    if ((vec->cur) + 1 > vec->max)
    {
        int new_max = vec->max;
        if (new_max > 1)
            new_max *= 1.5;
        else
            new_max += 1;
        int* pid_result = (int *)realloc(vec->pid_data, new_max * sizeof(int));
        option_t* command_result = (option_t*)realloc(vec->command_data, new_max * sizeof(option_t));
        if (pid_result == NULL || command_result == NULL)
            return false;
        vec->pid_data = pid_result;
        vec->command_data = command_result;
        vec->max = new_max;
    }
    vec->pid_data[vec->cur] = pid;
    vec->command_data[vec->cur] = command;
    vec->cur++;
    return true;
}

int simpsh_command_vector_get_count(command_vector_t vec)
{
    if (vec == NULL)
        return -1;
    return vec->cur;
}

void simpsh_command_vector_remove_by_index(command_vector_t vec, int index)
{
    if (vec == NULL || vec->cur == 0)
        return;
    if (index >= vec->cur)
        return;
    for (int i = index; i < (vec->cur) - 1; i++)
    {
        vec->pid_data[i] = vec->pid_data[i + 1];
        vec->command_data[i] = vec->command_data[i + 1];
    }
    vec->cur--;
}

void simpsh_command_vector_delete(command_vector_t vec)
{
    if (vec == NULL)
        return;
    free(vec->pid_data);
    free(vec->command_data);
    free(vec);
}
