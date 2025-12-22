#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_PATH "/home/zlgmcu/project/learnC++/log/log"
#define FILE_PATH_MAX_LEN 128 /* 文件路径最大缓冲长度 */
#define SYS_CMD_MAX_LEN 128

typedef enum
{
    PCU_ERR_INVAL = -2,  /* 入参错误 */
    PCU_ERR = -1,        /* 通用错误 */
    PCU_ERR_SUCCESS = 0, /* 成功 */
} pcu_err_code_e;


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

uint64_t gettime_msec(void);

/**
 * @brief 递归创建目录
 *
 * @param path 目录路径
 * @return int32_t
 */
int32_t make_dir_recursive(const char *path);


int32_t core_dump_file(bool enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __UTILS_H__ */
