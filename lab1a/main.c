#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "error.h"
#include "options.h"

int append_flag;
int cloexec_flag;
int creat_flag;
int directory_flag;
int dsync_flag;
int excl_flag;
int nofollow_flag;
int nonblock_flag;
int rsync_flag;
int sync_flag;
int trunc_flag;

int main(int argc, char** argv)
{
    zero_flags();
    int c;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if (c == -1)
            break;
        switch (c)
        {
            case 0:
                if (long_options[option_index].flag != 0)
                    break; //Flag was set
                break;
            case '?':
                break; //getopt_long already printed error msg
            default:
                //pass args to create_option and then check
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                {
                    printf(" with arg %s", optarg);
                    printf("\n");
                    if (optind < argc)
                    {
                        int temp = optind;
                        printf ("non-option ARGV-elements: ");
                        while (temp < argc)
                            printf ("%s ", argv[temp++]);
                        putchar ('\n');
                    }
                    continue;
                }
                putchar('\n');
        }
    }
    return 0;
}
