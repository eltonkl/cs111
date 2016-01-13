#ifndef HANDLERS_H
#define HANDLERS_H
#include "options.h"

void zero_flags();

//Function for --rdonly, --wronly, --and rdwr
int simpsh_open(const char* file, int setting);

#define SIMPSH_HANDLER(x) void simpsh_##x(option_t opt)

SIMPSH_HANDLER(rdonly);
SIMPSH_HANDLER(rdwr);
SIMPSH_HANDLER(wronly);
SIMPSH_HANDLER(pipe);
SIMPSH_HANDLER(command);
SIMPSH_HANDLER(wait);
SIMPSH_HANDLER(verbose);
SIMPSH_HANDLER(profile);
SIMPSH_HANDLER(abort);
SIMPSH_HANDLER(catch);
SIMPSH_HANDLER(ignore);
SIMPSH_HANDLER(default);
SIMPSH_HANDLER(pause);

extern void (*simpsh_handlers[NUM_ACTIONABLE_OPTIONS])(option_t);

#endif
