#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "simpsh.h"

static int  simpsh_max_fds;
static int* simpsh_fds;
static int  simpsh_num_fds;

void simpsh_init_fd_storage()
{
    simpsh_num_fds = 0;
    simpsh_fds = (int *)malloc(sizeof(int));
    if (!simpsh_fds)
    {
        fprintf(stderr, "Failed to allocate storage for file descriptors\n");
        simpsh_max_fds = 0;
    }
    else
        simpsh_max_fds = 1;
}

bool simpsh_get_fd(int number, int* fd_storage)
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
    return true;
}

void simpsh_add_fd(int fd)
{
    simpsh_num_fds++;
    if (simpsh_num_fds > simpsh_max_fds)
    {
        if (simpsh_max_fds > 1)
            simpsh_max_fds *= 1.5;
        else
            simpsh_max_fds += 1;
        int* result = (int *)realloc(simpsh_fds, simpsh_max_fds * sizeof(int));
        if (result == NULL)
        {
            fprintf(stderr, "Failed to allocate storage for file descriptors\n");
            return;
        }
        simpsh_fds = result;
        for (int i = simpsh_num_fds; i < simpsh_max_fds; i++)
            simpsh_fds[i] = -1;
    }
    simpsh_fds[simpsh_num_fds - 1] = fd;
}

void simpsh_finish_fd_storage()
{
    for (int i = 0; i < simpsh_num_fds; i++)
    {
        if (simpsh_fds[i] != -1)
        {
            if (close(simpsh_fds[i]) == -1)
                fprintf(stderr, "Failed to close a file\n");
        }
    }
    free(simpsh_fds);
}
