#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "simpsh.h"
#include "handlers.h"

int         simpsh_max_status;
bool        simpsh_print_verbose;

int         simpsh_max_fds;
int         simpsh_max_commands;
int         simpsh_num_fds;
int         simpsh_num_commands;
fd_t*       simpsh_fds;
command_t*  simpsh_commands;

bool is_exact_copy(const char* a, const char* b)
{
    while (*a != '\0' && *b != '\0')
    {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    if (*a != '\0' || *b != '\0')
        return false;
    return true;
}

void simpsh_init_fd_pid_storage(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (is_exact_copy("--command", argv[i]))
            simpsh_max_commands++;
        else if (is_exact_copy("--rdonly", argv[i]) ||
                 is_exact_copy("--wronly", argv[i]) ||
                 is_exact_copy("--rdwr", argv[i]))
            simpsh_max_fds++;
        else if (is_exact_copy("--pipe", argv[i]))
            simpsh_max_fds += 2;
    }
    simpsh_commands = (command_t*)malloc(sizeof(command_t) * simpsh_max_commands);
    simpsh_fds = (fd_t*)malloc(sizeof(fd_t) * simpsh_max_fds);
    if (simpsh_commands == NULL || simpsh_fds == NULL)
        fprintf(stderr, "Failed to allocate memory for pids or fds\n");
}

void simpsh_add_fd(int fd, simpsh_file_type type)
{
    simpsh_fds[simpsh_num_fds].fd = fd;
    simpsh_fds[simpsh_num_fds].type = type;
    simpsh_num_fds++;
}

void simpsh_add_command(int pid, option_t command, bool done)
{
    simpsh_commands[simpsh_num_commands].pid = pid;
    simpsh_commands[simpsh_num_commands].command = command;
    simpsh_commands[simpsh_num_commands].done = done;
    simpsh_num_commands++;
}

bool simpsh_get_fd(int index, fd_t* storage)
{
    if (index < 0 || index >= simpsh_num_fds || 
        simpsh_fds == NULL || simpsh_fds[index].type == SIMPSH_CLOSED)
    {
        fd_t fd = { -1, SIMPSH_CLOSED };
        *storage = fd;
        return false;
    }
    if (storage)
        *storage = simpsh_fds[index];
    return true;
}

bool simpsh_get_command_by_index(int index, command_t* storage)
{
    if (index < 0 || index >= simpsh_num_commands)
        return false;
    if (simpsh_commands == NULL)
        return false;
    if (storage)
        *storage = simpsh_commands[index];
    return true;
}

bool simpsh_get_command_by_pid(int pid, command_t* storage)
{
    for (int i = 0; i < simpsh_num_commands; i++)
    {
        if (simpsh_commands[i].pid == pid)
        {
            if (storage)
                *storage = simpsh_commands[i];
            return true;
        }
    }
    return false;
}

void simpsh_invalidate_fd(int index)
{
    if (index < 0 || index >= simpsh_num_fds)
        return;
    if (simpsh_fds == NULL)
        return;
    if (simpsh_fds[index].type == SIMPSH_CLOSED)
        return;
    close(simpsh_fds[index].fd);
    simpsh_fds[index].type = SIMPSH_CLOSED;
}

void simpsh_invalidate_command_by_index(int index)
{
    if (index < 0 || index >= simpsh_num_commands)
        return;
    if (simpsh_commands == NULL)
        return;
    simpsh_commands[index].done = true;
}

void simpsh_invalidate_command_by_pid(int pid)
{
    for (int i = 0; i < simpsh_num_commands; i++)
    {
        if (simpsh_commands[i].pid == pid)
            simpsh_commands[i].done = true;
    }
}

void simpsh_init(int argc, char** argv)
{
    zero_flags();
    simpsh_init_fd_pid_storage(argc, argv);
}

void simpsh_finish()
{
    for (int i = 0; i < simpsh_num_fds; i++)
        simpsh_invalidate_fd(i);

    for (int i = 0; i < simpsh_num_commands; i++)
    {
        command_t command;
        simpsh_get_command_by_index(i, &command);
        if (command.done)
            continue;
        int status;
        if (waitpid(command.pid, &status, 0) == command.pid)
        {
            if (WEXITSTATUS(status) > simpsh_max_status)
                simpsh_max_status = WEXITSTATUS(status);
            simpsh_invalidate_command_by_index(i);
        }
    }

    free(simpsh_commands);
    free(simpsh_fds);
}
