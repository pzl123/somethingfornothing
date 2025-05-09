#ifndef __PREPARED_STMT_H__
#define __PREPARED_STMT_H__

#include "sqlite3/sqlite3.h"
#include "database/dao/dsn.h"
#include "utils/utils.h"
#include "utils/cache/lru.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    sqlite3 *db;
    db_dsn_e dsn;
    lru_cache_t *cache;
    pthread_mutex_t mutex;
}prepared_stmt_t;

/**
 * @brief 初始化 prepared_stmt_t 结构
 * @param ps 指向 prepared_stmt_t 结构的指针
 * @param db 指向 sqlite3 数据库连接的指针
 * @param dsn 数据源标识符
 * @return 成功返回 true 失败返回 false
 */
bool new_prepared_stmt(prepared_stmt_t *ps, sqlite3 *db, db_dsn_e dsn);

/**
 * @brief 释放 prepared_stmt_t 结构及其相关资源
 * @param ps 指向 prepared_stmt_t 结构的指针
 */
void delete_prepared_stmt(prepared_stmt_t *ps);

/**
 * @brief 从缓存中获取 sqlite3_stmt 结构 如果不存在则创建新的 get 与 put必须成对使用
 * @param ps 指向 prepared_stmt_t 结构的指针
 * @param key SQL 语句字符串
 * @return 成功返回 sqlite3_stmt 结构指针 失败返回 NULL
 */
sqlite3_stmt* prepared_stmt_get(prepared_stmt_t *ps, const char *key);

/**
 * @brief 将 sqlite3_stmt 结构放回缓存 get 与 put必须成对使用
 * @param ps 指向 prepared_stmt_t 结构的指针
 * @param pstmt 指向 sqlite3_stmt 结构的指针
 */
void prepared_stmt_put(prepared_stmt_t *ps, sqlite3_stmt *pstmt);

#ifdef __cplusplus
}
#endif

#endif /* __PREPARED_STMT_H__ */
