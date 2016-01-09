#ifndef FDS_H
#define FDS_H

void simpsh_init_fd_storage();
bool get_fd(int number, int* fd_storage);
void add_fd(int fd);

#endif
