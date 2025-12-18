#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 获取文件大小
 *
 * @param file_path 文件路径
 * @return uint32_t 文件大小
 */
uint32_t file_size(const char* file_path);

/**
 * @brief 获取文件数据
 *
 * @param file_path 绝对路径
 * @param data 数据（动态内存，需要外部释放）
 * @param len 数据长度
 * @return int32_t 成功返回 PCU_ERR_SUCCESS，失败返回 PCU_ERR
 */
int32_t get_file_data(const char *file_path, uint8_t **data, uint32_t *len);

/**
 * @brief 文件复制
 *
 * @param dst 目标文件
 * @param src 源文件
 * @return bool 成功返回 true，失败返回 false
 */
bool file_copy(const char *dst, const char *src);

/**
 * @brief 文件移动
 *
 * @param dst 目标文件
 * @param src 源文件
 * @return bool 成功返回 true，失败返回 false
 */
bool file_move(const char *dst, const char *src);

/**
 * @brief 文件删除
 *
 * @param dst 目标文件
 * @return bool 成功返回 true，失败返回 false
 */
bool file_remove(const char *dst);

/**
 * @brief 文件是否存在
 *
 * @param path 路径
 * @return bool 存在返回 true，不存在返回 false
 */
bool file_exist(const char *path);


/**
 * @brief 获取目录大小，单位：KB
 */
int64_t dir_size(const char *path);

/**
 * @brief 是否是目录
 */
bool is_directory(const char *path);

/**
 * @brief 从路径中获取目录
 * @return 返回目录
 */
char *extract_directory(const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FILE_UTILS_H */
