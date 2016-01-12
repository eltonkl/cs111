#ifndef _GNU_SOURCE //Fix for testing on this Ubuntu system
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //close, fork, getopt_long, pipe, dup2, execvp
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>      //open and creat

#include "handlers.h"
#include "options.h"
#include "simpsh.h"

//Lookup table for function handlers
//The function pointer in our option_t struct will be set to one of these
//In this way, we can call whatever function we need in this table and simply implement as we go 
void (*simpsh_handlers[])(option_t opt) =
{
    simpsh_rdonly,
    simpsh_rdwr,
    simpsh_wronly,
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

SIMPSH_HANDLER(rdonly)
{
    int fd = simpsh_open(opt.args[0], O_RDONLY);
    if (fd == -1)
        fprintf(stderr, "Failed to open read only file %s\n", opt.args[0]);
    simpsh_add_fd(fd, SIMPSH_FILE);
}

SIMPSH_HANDLER(wronly)
{
    int fd = simpsh_open(opt.args[0], O_WRONLY);
    if (fd == -1)
        fprintf(stderr, "Failed to open write only file %s\n", opt.args[0]);
    simpsh_add_fd(fd, SIMPSH_FILE);
}

SIMPSH_HANDLER(rdwr)
{
    int fd = simpsh_open(opt.args[0], O_RDWR);
    if (fd == -1)
        fprintf(stderr, "Failed to open read/write file %s\n", opt.args[0]);
    simpsh_add_fd(fd, SIMPSH_FILE);
}

SIMPSH_HANDLER(pipe)
{
    (void)opt;
    int fds[2];
    if (pipe(fds) == -1)
    {
        fprintf(stderr, "Failed to create pipe\n");
        fds[0] = -1;
        fds[1] = -1;
    }
    simpsh_add_fd(fds[0], SIMPSH_PIPE_READ);
    simpsh_add_fd(fds[1], SIMPSH_PIPE_WRITE);
}

SIMPSH_HANDLER(command)
{ 
    int stdin_logical_fd = (int)strtol(opt.args[0], NULL, 0);
    int stdout_logical_fd = (int)strtol(opt.args[1], NULL, 0);
    int stderr_logical_fd = (int)strtol(opt.args[2], NULL, 0);
    fd_t stdin_fd, stdout_fd, stderr_fd;
    simpsh_get_fd(stdin_logical_fd, &stdin_fd);
    simpsh_get_fd(stdout_logical_fd, &stdout_fd);
    simpsh_get_fd(stderr_logical_fd, &stderr_fd);
    
    //Fork the pipes and later account for potential bugs
    int pid = fork();
    if (pid == 0)
    {               
        if (stdin_fd.type == SIMPSH_PIPE_READ)
        {
            fd_t write_fd;
            if (simpsh_get_fd(stdin_logical_fd + 1, &write_fd))
            {
                if (close(write_fd.fd) == -1)
                    fprintf(stderr, "Failed to close write pipe\n");
            }
            simpsh_invalidate_fd(stdin_logical_fd + 1);
        }
        else if (stdin_fd.type == SIMPSH_PIPE_WRITE)
            fprintf(stderr, "Warning: attempt to assign write pipe to stdin\n");
        if (stdout_fd.type == SIMPSH_PIPE_WRITE)
        {
            fd_t read_fd;
            if (simpsh_get_fd(stdout_logical_fd - 1, &read_fd))
            {
                if (close(read_fd.fd) == -1)
                    fprintf(stderr, "Failed to close read pipe\n");
            }
            simpsh_invalidate_fd(stdout_logical_fd - 1);
        }
        else if (stdout_fd.type == SIMPSH_PIPE_READ)
            fprintf(stderr, "Warning: attempt to assign read pipe to stdout\n");
        if (stderr_fd.type == SIMPSH_PIPE_WRITE)
        {
            fd_t read_fd;
            if (simpsh_get_fd(stderr_logical_fd - 1, &read_fd))
            {
                if (close(read_fd.fd) == -1)
                    fprintf(stderr, "Failed to close read pipe\n");
            }
            simpsh_invalidate_fd(stderr_logical_fd - 1);
        }
        else if (stderr_fd.type == SIMPSH_PIPE_READ)
            fprintf(stderr, "Warning: attempt to assign read pipe to stderr\n");
        
        if (dup2(stdin_fd.fd, 0) == -1)
        {
            fprintf(stderr, "Failed to redirect stdin\n");
            exit(1);
        }
        if (dup2(stdout_fd.fd, 1) == -1)
        {
            fprintf(stderr, "Failed to redirect stdout\n");
            exit(1);
        }
        if (dup2(stderr_fd.fd, 2) == -1)
        {
            fprintf(stderr, "Failed to redirect stderr\n");
            exit(1);
        }
        close(stdin_fd.fd);
        close(stdout_fd.fd);
        close(stderr_fd.fd);

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
            exit(1);
    }
    else
    {
        //If the parent executes a command that uses a pipe, it needs to close the end it gives away to the child
        //So, in order to prevent pipes from stalling the execution of the program we close them
        //We invalidate after this so that the child's pipe end is safe from other attempts
        if (stdin_fd.type == SIMPSH_PIPE_READ)
        {
            close(stdin_fd.fd);
            simpsh_invalidate_fd(stdin_logical_fd);
        }
        if (stdout_fd.type == SIMPSH_PIPE_WRITE)
        {
            close(stdout_fd.fd);
            simpsh_invalidate_fd(stdout_logical_fd);
        }
        if (stderr_fd.type == SIMPSH_PIPE_WRITE)
        {
            close(stderr_fd.fd);
            simpsh_invalidate_fd(stderr_logical_fd);
        }
        simpsh_add_command(pid, opt, false);
    }
}

SIMPSH_HANDLER(wait)
{
    (void)opt;
    int start_num_commands = 0;
    for (int i = 0; i < simpsh_num_commands; i++, start_num_commands++)
    {
        command_t command;
        simpsh_get_command(i, &command);
        if (!command.done)
            break;
    }

    for (int i = start_num_commands; i < simpsh_num_commands; i++)
    {
        command_t command;
        simpsh_get_command(i, &command);
        int status;
        if (wait(&status) != -1)
        {
            if (WEXITSTATUS(status) > simpsh_max_status)
                simpsh_max_status = WEXITSTATUS(status);
            printf("%i %s", WEXITSTATUS(status), command.command.args[3]);
            if (command.command.num_args > 4)
            {
                for (int i = 4; i < command.command.num_args; i++)
                    printf(" %s", command.command.args[i]);
            }
            putchar('\n');
            simpsh_invalidate_command(i);
        }
    }
}

SIMPSH_HANDLER(verbose)
{
    (void)opt;
    simpsh_print_verbose = true;
}

SIMPSH_HANDLER(profile)
{
    (void)opt;
}

SIMPSH_HANDLER(abort)
{
    (void)opt;
    *(volatile int *)NULL = 0;
}

static void catch_handler(int signo)
{
    fprintf(stderr, "%i caught\n", signo);
    exit(signo);
}

SIMPSH_HANDLER(catch)
{
    char* end;
    int num = (int)strtol(opt.args[0], &end, 0);
    if (end == opt.args[0])
    {
        fprintf(stderr, "Option \'--catch\' failed: not a valid number: %s\n", opt.args[0]);
        return;
    }
    struct sigaction sa;
    sa.sa_handler = catch_handler;
    if (sigaction(num, &sa, NULL) == -1)
        fprintf(stderr, "Failed to install signal handler for signal %i\n", num);
}

SIMPSH_HANDLER(ignore)
{
    char* end;
    int num = (int)strtol(opt.args[0], &end, 0);
    if (end == opt.args[0])
    {
        fprintf(stderr, "Option \'--ignore\' failed: not a valid number: %s\n", opt.args[0]);
        return;
    }
    if (signal(num, SIG_IGN) == SIG_ERR)
        fprintf(stderr, "Failed to ignore signal %i\n", num);
}

SIMPSH_HANDLER(default)
{
    char* end;
    int num = (int)strtol(opt.args[0], &end, 0);
    if (end == opt.args[0])
    {
        fprintf(stderr, "Option \'--default\' failed: not a valid number: %s\n", opt.args[0]);
        return;
    }
    if (signal(num, SIG_DFL) == SIG_ERR)
        fprintf(stderr, "Failed to reset signal handler to default behavior for signal %i\n", num);
}

SIMPSH_HANDLER(pause)
{
    (void)opt;
    pause();
}
