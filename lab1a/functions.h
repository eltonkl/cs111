#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "options.h"

void zero_flags();

//Function for --rdonly, --wronly, --and rdwr
int simpsh_open(const char* file, int setting);

#define SIMPSH_FUNC(x) void simpsh_##x(option_t opt)

SIMPSH_FUNC(rdonly);
SIMPSH_FUNC(wronly);
SIMPSH_FUNC(rdwr);
SIMPSH_FUNC(pipe);
SIMPSH_FUNC(command);
SIMPSH_FUNC(wait);
SIMPSH_FUNC(verbose);
SIMPSH_FUNC(profile);
SIMPSH_FUNC(abort);
SIMPSH_FUNC(catch);
SIMPSH_FUNC(ignore);
SIMPSH_FUNC(default);
SIMPSH_FUNC(pause);

extern void (*simpsh_functions[NUM_ACTIONABLE_OPTIONS])(option_t);

#endif
