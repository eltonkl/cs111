#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//Function for --rdonly, --wronly, --and rdwr
int simpsh_open(const char* file, int setting);

//Function for --abort
void simpsh_abort();

//Function for --verbose
void simpsh_verbose_enable();

//Function for --profile

//Function for --catch N

//Function for --ignore N

//Function for --default N

#endif
