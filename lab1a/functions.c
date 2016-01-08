#include <unistd.h>                                                 //close, fork, getopt_long, pipe, dup2, execvp
#include <sys/types.h> #include <sys/stat.h> #include <fcntl.h>     //open and creat
#include "functions.h"
#include "options.h"

int simpsh_open(const char* file, char * setting)
{
    int flags = 0;
    
    if (append_flag)
        flags |= O_APPEND;
    if (cloexec_flag)
        flags |= O_CLOEXEC;
    if (creat_flag)
        flags |= O_CREAT;
    if (directory_flag)
        flags |= O_DIRECTORY;
    if (dsync_flag)
        flags |= O_DSYNC;
    if (excl_flag)
        flags |= O_EXCL;
    if (nofollow_flag)
        flags |= O_NOFOLLOW;
    if (nonblock_flag)
        flags |= O_NONBLOCK;
    if (rsync_flag)
        flags |= O_RSYNC;
    if (sync_flag)
        flags |= O_SYNC;
    if (trunc_flag)
        flags |= O_TRUNC;

    //One of these must be included in the flags in order for the open function to work
    switch (setting)
    {
        case 'O_RDONLY':
            flags |= O_RDONLY;
            break;
        case 'O_WRONLY':
            flags |=  O_WRONLY;
            break;
        case 'O_RDWR':
            flags |= O_RDWR;
            break;
    }
    
    int fd = open(file, flags);
    return fd;
}

//Function for --abort
void simpsh_abort()
{
    int * x = NULL;
    int y = &x;
}

//Function for --verbose; return status of simpsh_verbose
void simpsh_verbose_enable()
{
    simpsh_verbose = true;
}