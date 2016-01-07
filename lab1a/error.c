#include <stdio.h>

#include "error.h"

void print_error(const char* error)
{
    fputs(error, stderr);
}
