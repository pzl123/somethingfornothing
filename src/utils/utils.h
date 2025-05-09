#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* 浮点数据类型重定义 */
typedef float  float32_t;
typedef double float64_t;

#define d_log(fmt, ...) \
    errif_debug(__LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

void errif_debug(int line, const char *file, pthread_t pid, const char *fmt, ...);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __UTILS_H__ */
