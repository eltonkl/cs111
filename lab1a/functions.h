#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//All functions that are called from here will be preceded by simpsh_ as a convention
//Function for --rdonly, --wronly, --and rdwr
int simpsh_open(const char* file, char * setting);

//Function for --abort
void simpsh_abort();

//Function for --verbose
//Enables verbose mode for the duration of this particular execution
void simpsh_verbose_enable();

//Function for --profile

//Function for --catch N

//Function for --ignore N

//Function for --default N

#endif