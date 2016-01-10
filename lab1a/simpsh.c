#include "simpsh.h"
#include "fds.h"
#include "functions.h"
#include "vector.h"

int simpsh_max_status;
bool simpsh_print_verbose;
vector_t simpsh_pids;

void simpsh_init()
{
    zero_flags();
    simpsh_init_fd_storage();
    simpsh_pids = simpsh_vector_create();
    simpsh_max_status = 0;
    simpsh_print_verbose = false;
}

void simpsh_finish()
{
    simpsh_delete_fd_storage();
    simpsh_vector_delete(simpsh_pids);
}
