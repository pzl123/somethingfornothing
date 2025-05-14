#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_PATH "/home/zlgmcu/project/learnC++/log/log"

#define d_log(fmt, ...) \
    errif_debug(__LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

void errif_debug(int line, const char *file, pthread_t pid, const char *fmt, ...);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __UTILS_H__ */
