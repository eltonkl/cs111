#include <stdio.h>
#include <stdbool.h>

#include "options.h"

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    char *args[] = { "--abort", "56" };
    option_t* opt = create_option(2, args);
    bool result = is_valid_option(opt); 
    return 0;
}
