#ifndef FDS_H
#define FDS_H

typedef enum simpsh_file_type
{
    SIMPSH_FILE, 
    SIMPSH_PIPE_READ, 
    SIMPSH_PIPE_WRITE,
    SIMPSH_ERROR
} simpsh_file_type;

void simpsh_init_fd_storage();
void simpsh_delete_fd_storage();
bool simpsh_get_fd(int number, int* fd_storage, simpsh_file_type* type_storage);
void simpsh_invalidate_fd(int number);
void simpsh_add_fd(int fd, simpsh_file_type type);

#endif
