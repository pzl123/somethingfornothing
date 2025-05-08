#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

void errif_debug(int line, const char *file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

    const char *filename = strrchr(file, '/');
    if (filename == NULL) filename = file;
    else filename += 1;

    fprintf(stderr, "\x1b[34m[DEBUG]\x1b[0m [%s %s:%d]: ", time_str, filename, line);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}