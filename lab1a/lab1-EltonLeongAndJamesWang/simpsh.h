#ifndef SIMPSH_H
#define SIMPSH_H

#include <setjmp.h>
#include <stdbool.h>

#include "options.h"

#define SIMPSH_MAX(a,b) ((a > b) ? a : b) 

extern jmp_buf          simpsh_context;
extern int              simpsh_last_signal;
extern int              simpsh_max_status;
extern bool             simpsh_print_verbose;

typedef struct command_t
{
    int pid;
    option_t command;
    bool done;
} command_t;

typedef enum simpsh_file_type
{
    SIMPSH_FILE, 
    SIMPSH_PIPE_READ, 
    SIMPSH_PIPE_WRITE,
    SIMPSH_CLOSED
} simpsh_file_type;

typedef struct fd_t
{
    int fd;
    simpsh_file_type type;
} fd_t;

extern int          simpsh_max_fds;
extern int          simpsh_max_commands;
extern int          simpsh_num_fds;
extern int          simpsh_num_commands;
extern fd_t*        simpsh_fds;
extern command_t*   simpsh_commands;

void simpsh_error_set_status();
void simpsh_add_fd(int fd, simpsh_file_type type);
void simpsh_add_command(int pid, option_t command, bool done);
bool simpsh_get_fd(int index, fd_t* storage);
bool simpsh_get_command_by_index(int index, command_t* storage);
bool simpsh_get_command_by_pid(int pid, command_t* storage);
bool simpsh_invalidate_fd(int index);
void simpsh_invalidate_command_by_pid(int pid);

extern int append_flag;
extern int cloexec_flag;
extern int creat_flag;
extern int directory_flag;
extern int dsync_flag;
extern int excl_flag;
extern int nofollow_flag;
extern int nonblock_flag;
extern int rsync_flag;
extern int sync_flag;
extern int trunc_flag;

void simpsh_init(int argc, char** argv);
void simpsh_finish();

#endif
