#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "simpsh.h"

static int  simpsh_max_fds;
static int* simpsh_fds;
static int  simpsh_num_fds;

void simpsh_init_fd_storage()
{
    simpsh_num_fds = 0;
    simpsh_fds = malloc(sizeof(int));
    if (!simpsh_fds)
    {
        fprintf(stderr, "Failed to allocate storage for file descriptors");
        simpsh_max_fds = 0;
    }
    else
        simpsh_max_fds = 1;
}

bool simpsh_get_fd(int number, int* fd_storage)
{
    if (number < 0 || number > simpsh_num_fds)
        return false;
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
        int* result = realloc(simpsh_fds, simpsh_max_fds * sizeof(int));
        if (result == NULL)
        {
            fprintf(stderr, "Failed to allocate storage for file descriptors");
            return;
        }
        simpsh_fds = result;
    }
    simpsh_fds[simpsh_num_fds - 1] = fd;
}
