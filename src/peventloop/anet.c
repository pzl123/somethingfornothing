#include "anet.h"

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* 为指定的文件描述符（file descriptor, fd）设置 FD_CLOEXEC 标志，
以防止在调用 fork() + execve() 时该描述符被意外继承到子进程中 */
#define UNUSED(x) (void)(x)

int anetCloexec(int fd)
{
    int r;
    int flags;
    do
    {
        r = fcntl(fd, F_GETFD);
    } while ((-1 == r) && EINTR == errno);

    if ((-1 == r) || (r & FD_CLOEXEC))
    {
        return r;
    }
    flags = r | FD_CLOEXEC;
    do
    {
        r = fcntl(fd, F_SETFD, flags);
    } while ((-1 == r) && (EINTR == errno));

    return r;
}