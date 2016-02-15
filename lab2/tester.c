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
    int success = 1;
    int devfd = open(devname, O_RDONLY);
    int devfd2;
    if (ioctl(devfd, OSPRDIOCACQUIRE, NULL) == -1)
    {
        success = 0;
        perror("Failed to acquire read lock 1");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCRELEASE, NULL) == -1)
    {
        success = 0;
        perror("Failed to release read lock 1");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCACQUIRE, NULL) == -1)
    {
        success = 0;
        perror("Failed to acquire read lock 2");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCACQUIRE, NULL) == -1)
    {
        success = 0;
        perror("Failed to acquire read lock 3");
        goto end;
    }
    devfd2 = open(devname, O_WRONLY);
    if (ioctl(devfd2, OSPRDIOCTRYACQUIRE, NULL) != -1)
    {
        success = 0;
        perror("Successfully try-acquired a write lock with preexisting read locks 2 and 3");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCRELEASE, NULL) == -1)
    {
        success = 0;
        perror("Failed to release read lock 2");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCRELEASE, NULL) == -1)
    {
        success = 0;
        perror("Failed to release read lock 3");
        goto end;
    }
    if (ioctl(devfd2, OSPRDIOCTRYACQUIRE, NULL) == -1)
    {
        success = 0;
        perror("Failed to try-acquire write lock 1");
        goto end;
    }
    if (ioctl(devfd, OSPRDIOCTRYACQUIRE, NULL) != -1)
    {
        success = 0;
        perror("Successfully try-acquired a read lock with preexisting write lock 1");
        goto end;
    }
    close(devfd2);
    if (ioctl(devfd, OSPRDIOCACQUIRE, NULL) == -1)
    {
        success = 0;
        perror("Failed to acquire a read lock after closing write fd to ramdisk");
        goto end;
    }
    close(devfd);
end:
    if (success)
        printf("%d", success);
    return 0;
}
