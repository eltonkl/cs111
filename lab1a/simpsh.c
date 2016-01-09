#include "simpsh.h"
#include "fds.h"
#include "functions.h"

int simpsh_max_status;
bool simpsh_print_verbose;

void simpsh_init()
{
    zero_flags();
    simpsh_init_fd_storage();
    simpsh_max_status = 0;
    simpsh_print_verbose = false;
}
