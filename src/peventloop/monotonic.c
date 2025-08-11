#include "monotonic.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* 时钟检索的函数指针 */
monotime (*getMonotonicUs)(void) = NULL;

static char monotonic_info_string[32];

static monotime getMonotonicUs_posix(void)
{
    /* clock_gettime() 是在 POSIX.1b 标准（1993 年）中定义的。尽管如此，一些系统直到更晚的版本才开始支持它。
    CLOCK_MONOTONIC 在技术上是可选的，可能不被支持——但目前看来它已经得到了普遍支持。
    如果系统不支持 CLOCK_MONOTONIC，请提供一个特定于该系统的替代实现。 */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}

static void monotonicInit_posix(void)
{
    /* 确保支持CLOCK_MONONIC。这应该得到当前操作系统的支持。如果下面的断言失败，请提供适当的替代实现 */
    struct timespec ts;
    int rc = clock_gettime(CLOCK_MONOTONIC, &ts);
    snprintf(monotonic_info_string, sizeof(monotonic_info_string),
            "POSIX clock_gettime");
    getMonotonicUs = getMonotonicUs_posix;
}

const char *monotonicInit(void)
{
    if (getMonotonicUs == NULL) monotonicInit_posix();
    return monotonic_info_string;
}

const char *monotonicInfoString(void)
{
    return monotonic_info_string;
}

monotonic_clock_e monotonicGetType(void)
{
    if (getMonotonicUs == getMonotonicUs_posix)
        return MONOTONIC_CLOCK_POSIX;
    return MONOTONIC_CLOCK_HW;
}
