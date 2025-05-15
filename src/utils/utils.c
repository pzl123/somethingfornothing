#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>



typedef enum
{
    DEBUG_LEVEL = 0,
    INFO_LEVEL,
    WARN_LEVEL,
    ERROR_LEVEL,
    DEFAULT_LEVEL
} log_level_e;

#ifndef LOG_LEVEL
#define LOG_LEVEL DEFAULT_LEVEL  // 默认级别为 0
#endif

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void write_frm_td(char* str, log_level_e level)
{
    if (LOG_LEVEL < level)
    {
        (void)str;
        (void)level;
        (void)mutex;
        return;
    }
    else
    {
        (void)level;
#ifdef LOG_STDOUT
        pthread_mutex_lock(&mutex);
        fprintf(stdout, "%s", str);
        fflush(stdout);
        pthread_mutex_unlock(&mutex);
        return;
#else
        pthread_mutex_lock(&mutex);
        FILE *fp;
        fp = fopen(LOG_PATH,"a");
        fwrite(str, 1, strlen(str), fp);
        fflush(fp);
        fclose(fp);
        pthread_mutex_unlock(&mutex);
        return;
#endif
    }

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

void errif_debug(const char *type, int line, const char *file, pthread_t pid, const char *fmt, ...)
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

    log_level_e level = DEBUG_LEVEL;
// 设置颜色前缀
    const char *color_prefix = "";
    const char *color_suffix = "\x1b[0m";
    if (strcmp(type, "DEBUG") == 0)
    {
        color_prefix = "\x1b[34m"; // Blue
        level = DEBUG_LEVEL;
    }
    else if (strcmp(type, "INFO") == 0)
    {
        color_prefix = "\x1b[32m"; // Green
        level = INFO_LEVEL;
    }
    else if (strcmp(type, "WARN") == 0)
    {
        color_prefix = "\x1b[35m"; // Purple
        level = WARN_LEVEL;
    }
    else if (strcmp(type, "ERROR") == 0)
    {
        color_prefix = "\x1b[31m"; // Red
        level = ERROR_LEVEL;
    }
#ifdef LOG_STDOUT
    // 构建日志头
    snprintf(log_buffer, sizeof(log_buffer),"%s[%-5s]%s [%s %lx %s:%d]: ",
             color_prefix, type, color_suffix,time_str, pid, filename, line);
    vsnprintf(log_buffer + strlen(log_buffer), sizeof(log_buffer) - strlen(log_buffer), fmt, args);
#else
    (void)color_prefix;
    (void)color_suffix;
    snprintf(log_buffer, sizeof(log_buffer), "[%-5s] [%s %lx %s:%d]: ", type, time_str, pid, filename, line);
    vsnprintf(log_buffer + strlen(log_buffer), sizeof(log_buffer) - strlen(log_buffer), fmt, args);
#endif
    strcat(log_buffer, "\n");
    write_frm_td(log_buffer, level);
    va_end(args);
}

void log_init(void)
{
    FILE *fp  = fopen(LOG_PATH, "a+");
    fclose(fp);
}
