#include <unistd.h>                                                 //close, fork, getopt_long, pipe, dup2, execvp
#include <sys/types.h> #include <sys/stat.h> #include <fcntl.h>     //open and creat
#include "functions.h"

int rdonly(const char* file)
{
    int fd = open(file, O_RDONLY);
    return fd;
}

int wronly(const char* file)
{
    int fd = open(file, O_WRONLY);
    return fd;
}
