#ifndef _GNU_SOURCE //Fix for testing on this Ubuntu system
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //close, fork, getopt_long, pipe, dup2, execvp
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>      //open and creat

#include "functions.h"
#include "options.h"
#include "simpsh.h"
#include "fds.h"

void (*simpsh_functions[])(option_t opt) =
{
    simpsh_rdonly,
    simpsh_wronly,
    simpsh_rdwr,
    simpsh_pipe,
    simpsh_command,
    simpsh_wait,
    simpsh_verbose,
    simpsh_profile,
    simpsh_abort,
    simpsh_catch,
    simpsh_ignore,
    simpsh_default,
    simpsh_pause,
};

int append_flag;
int cloexec_flag;
int creat_flag;
int directory_flag;
int dsync_flag;
int excl_flag;
int nofollow_flag;
int nonblock_flag;
int rsync_flag;
int sync_flag;
int trunc_flag;

void zero_flags()
{
    append_flag = 0;
    cloexec_flag = 0;
    creat_flag = 0;
    directory_flag = 0;
    dsync_flag = 0;
    excl_flag = 0;
    nofollow_flag = 0;
    nonblock_flag = 0;
    rsync_flag = 0;
    sync_flag = 0;
    trunc_flag = 0;
}

int simpsh_open(const char* file, int setting)
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

    //setting must be one of O_RDONLY, O_WRONLY, O_RDWR or this function fails
    flags |= setting;

    int fd = open(file, flags, 0644);
    zero_flags();
    return fd;
}

SIMPSH_FUNC(rdonly)
{
    int fd = simpsh_open(opt.args[0], O_RDONLY);
    if (fd == -1)
        fprintf(stderr, "Failed to open read only file %s\n", opt.args[0]);
    simpsh_add_fd(fd);
}

SIMPSH_FUNC(wronly)
{
    int fd = simpsh_open(opt.args[0], O_WRONLY);
    if (fd == -1)
        fprintf(stderr, "Failed to open write only file %s\n", opt.args[0]);
    simpsh_add_fd(fd);
}

SIMPSH_FUNC(rdwr)
{
    int fd = simpsh_open(opt.args[0], O_RDWR);
    if (fd == -1)
        fprintf(stderr, "Failed to open read/write file %s\n", opt.args[0]);
    simpsh_add_fd(fd);
}

SIMPSH_FUNC(pipe)
{
    (void)opt;
}

SIMPSH_FUNC(command)
{
    int pid = fork();
    if (pid == 0)
    {
        int new_stdin_logical_fd_num = (int)strtol(opt.args[0], NULL, 0);
        int new_stdout_logical_fd_num  = (int)strtol(opt.args[1], NULL, 0);
        int new_stderr_logical_fd_num = (int)strtol(opt.args[2], NULL, 0);
        int stdin_fd, stdout_fd, stderr_fd;
        simpsh_get_fd(new_stdin_logical_fd_num, &stdin_fd);
        simpsh_get_fd(new_stdout_logical_fd_num, &stdout_fd);
        simpsh_get_fd(new_stderr_logical_fd_num, &stderr_fd);
        if (dup2(stdin_fd, 0) == -1)
        {
            fprintf(stderr, "Failed to redirect stdin\n");
            exit(1);
        }
        if (dup2(stdout_fd, 1) == -1)
        {
            fprintf(stderr, "Failed to redirect stdout\n");
            exit(1);
        }
        if (dup2(stderr_fd, 2) == -1)
        {
            fprintf(stderr, "Failed to redirect stderr\n");
            exit(1);
        }

        int args_to_program = opt.num_args - 4;
        char** args = (char**)malloc(sizeof(char*) * (args_to_program + 2));
        if (!args)
        {
            fprintf(stderr, "Failed to alloc memory for args\n");
            exit(1);
        }
        args[0] = opt.args[3];
        for (int i = 0; i < args_to_program; i++)
            args[1 + i] = opt.args[4 + i];
        args[args_to_program + 1] = NULL;
        if (execvp(args[0], args) == -1)
        {
            fprintf(stderr, "execvp failed\n");
            exit(1);
        }
    }
}

SIMPSH_FUNC(wait)
{
    (void)opt;
}

SIMPSH_FUNC(verbose)
{
    (void)opt;
    simpsh_print_verbose = true;
}

SIMPSH_FUNC(profile)
{
    (void)opt;
}

SIMPSH_FUNC(abort)
{
    (void)opt;
    *(volatile int *)NULL = 0;
}

SIMPSH_FUNC(catch)
{
    (void)opt;
}

SIMPSH_FUNC(ignore)
{
    (void)opt;
}

SIMPSH_FUNC(default)
{
    (void)opt;
}

SIMPSH_FUNC(pause)
{
    (void)opt;
}
