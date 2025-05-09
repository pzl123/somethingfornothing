#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

void get_time_with_ms(char *time_str, size_t len)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(time_str, len, "%Y-%m-%d %H:%M:%S", tm_info);

    // 将毫秒部分追加到字符串中
    snprintf(time_str + strlen(time_str), len - strlen(time_str), ".%03d", (int)(tv.tv_usec / 1000));
}

void errif_debug(int line, const char *file, pthread_t pid, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char time_str[25];
    get_time_with_ms(time_str, sizeof(time_str)/sizeof(time_str[0]));

    const char *filename = strrchr(file, '/');
    if (filename == NULL) filename = file;
    else filename += 1;

    fprintf(stderr, "\x1b[34m[DEBUG]\x1b[0m [%s %lx %s:%d]: ", time_str, pid, filename, line);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}