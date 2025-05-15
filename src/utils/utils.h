#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_PATH "/home/zlgmcu/project/learnC++/log/log"

#define d_log(fmt, ...) \
    errif_debug("DEBUG", __LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

#define i_log(fmt, ...) \
        errif_debug("INFO", __LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

#define w_log(fmt, ...) \
    errif_debug("WARN", __LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

#define e_log(fmt, ...) \
    errif_debug("ERROR", __LINE__, __FILE__, pthread_self(), fmt, ##__VA_ARGS__)

void errif_debug(const char* type, int line, const char *file, pthread_t pid, const char *fmt, ...);

void log_init(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __UTILS_H__ */
