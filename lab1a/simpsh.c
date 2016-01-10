#include "simpsh.h"
#include "fds.h"
#include "functions.h"
#include "vector.h"

int simpsh_max_status;
bool simpsh_print_verbose;
command_vector_t simpsh_commands;

void simpsh_init()
{
    zero_flags();
    simpsh_init_fd_storage();
    simpsh_commands = simpsh_command_vector_create();
    simpsh_max_status = 0;
    simpsh_print_verbose = false;
}

void simpsh_finish()
{
    simpsh_delete_fd_storage();
    simpsh_command_vector_delete(simpsh_commands);
}
