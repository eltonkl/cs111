#ifndef FDS_H
#define FDS_H

void simpsh_init_fd_storage();
void simpsh_delete_fd_storage();
bool simpsh_get_fd(int number, int* fd_storage);
void simpsh_add_fd(int fd);

#endif
