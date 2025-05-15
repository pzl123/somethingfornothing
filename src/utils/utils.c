#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void write_frm_td(char* str)
{
    pthread_mutex_lock(&mutex);
    FILE *fp;
    fp = fopen(LOG_PATH,"a");
    fwrite(str, 1, strlen(str), fp);
    fflush(fp);
    fclose(fp);
    pthread_mutex_unlock(&mutex);
}

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

    // 构建日志内容
    char log_buffer[2048];
#ifdef LOG_STDOUT
    snprintf(log_buffer, sizeof(log_buffer), "\x1b[34m[DEBUG]\x1b[0m [%s %lx %s:%d]: ", time_str, pid, filename, line);
#else
    snprintf(log_buffer, sizeof(log_buffer), "[DEBUG] [%s %lx %s:%d]: ", time_str, pid, filename, line);
#endif
    vsnprintf(log_buffer + strlen(log_buffer), sizeof(log_buffer) - strlen(log_buffer), fmt, args);
    strcat(log_buffer, "\n");
    write_frm_td(log_buffer);


    va_end(args);
}

void log_init(void)
{
    FILE *fp  = fopen(LOG_PATH, "a+");
    fclose(fp);
}
