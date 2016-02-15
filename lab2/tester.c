#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "osprd.h"

int main()
{
    const char devname[] = "/dev/osprda";
    int mode = O_RDONLY;
    int success = 1;
    int devfd = open(devname, mode);
    if (ioctl(devfd, OSPRDIOCACQUIRE, NULL) == -1)
    {
        success = 0;
        printf("Failed to acquire a read lock\n");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCRELEASE, NULL) == -1)
    {
        success = 0;
        printf("Failed to release the read lock\n");
        goto end;
    }
end:
    if (success)
        printf("%d", success);
    return 0;
}
