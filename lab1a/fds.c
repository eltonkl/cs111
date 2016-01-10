#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "fds.h"
#include "simpsh.h"

static int                  simpsh_max_fds;
static int*                 simpsh_fds;
static int                  simpsh_num_fds;
static simpsh_file_type*    simpsh_fd_pipe_map;

void simpsh_init_fd_storage()
{
    simpsh_num_fds = 0;
    simpsh_fds = (int*)malloc(sizeof(int));
    simpsh_fd_pipe_map = (simpsh_file_type*)malloc(sizeof(simpsh_file_type));
    if (simpsh_fds == NULL || simpsh_fd_pipe_map == NULL)
    {
        fprintf(stderr, "Failed to allocate storage for file descriptors\n");
        simpsh_max_fds = 0;
    }
    else
        simpsh_max_fds = 1;
}

bool simpsh_get_fd(int number, int* fd_storage, simpsh_file_type* type_storage)
{
    if (number < 0 || number >= simpsh_num_fds)
        return false;
    if (number >= simpsh_max_fds) //this branch indicates that a realloc failed,
        //so we have more logical fds than we have real fds
    {
        fprintf(stderr, "Error: tried to retrieve a fd whose storage was never successfully allocated\n");
        if (fd_storage)
            *fd_storage = -1;
        return false;
    }
    if (fd_storage)
        *fd_storage = simpsh_fds[number];
    if (type_storage)
        *type_storage = simpsh_fd_pipe_map[number];
    return true;
}

void simpsh_invalidate_fd(int number)
{
    if (number < 0 || number >= simpsh_num_fds)
        return;
    simpsh_fds[number] = -1;
    simpsh_fd_pipe_map[number] = SIMPSH_ERROR;
}

void simpsh_add_fd(int fd, simpsh_file_type type)
{
    simpsh_num_fds++;
    if (simpsh_num_fds > simpsh_max_fds)
    {
        if (simpsh_max_fds > 1)
            simpsh_max_fds *= 1.5;
        else
            simpsh_max_fds += 1;
        int* result = (int*)realloc(simpsh_fds, simpsh_max_fds * sizeof(int));
        simpsh_file_type* type_result = (simpsh_file_type*)realloc(simpsh_fd_pipe_map, simpsh_max_fds * sizeof(simpsh_file_type));
        if (result == NULL || type_result == NULL)
        {
            fprintf(stderr, "Failed to allocate storage for file descriptors\n");
            return;
        }
        simpsh_fds = result;
        simpsh_fd_pipe_map = type_result;
        for (int i = simpsh_num_fds; i < simpsh_max_fds; i++)
        {
            simpsh_fds[i] = -1;
            simpsh_fd_pipe_map[i] = SIMPSH_ERROR;
        }
    }
    simpsh_fds[simpsh_num_fds - 1] = fd;
    simpsh_fd_pipe_map[simpsh_num_fds - 1] = type;
}

void simpsh_delete_fd_storage()
{
    for (int i = 0; i < simpsh_num_fds; i++)
    {
        if (simpsh_fds[i] != -1)
        {
            if (close(simpsh_fds[i]) == -1)
                fprintf(stderr, "Failed to close a file or pipe\n");
        }
    }
    free(simpsh_fds);
    free(simpsh_fd_pipe_map);
}
