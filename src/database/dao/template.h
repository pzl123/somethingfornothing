#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include "utils/utils.h"
#include "sqlite3/sqlite3.h"
#include "database/dao/prepared_stmt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAO_RETURN_IS_OK(func)                      \
if (SQLITE_OK != func)                              \
{                                                   \
    d_log("sql_err=%s", sqlite3_errmsg(db));  \
}

typedef void (*stmt_bind_cb)(sqlite3*, sqlite3_stmt*, void*);

/**
 * @brief 批量绑定 sql 语句的回调函数
 * @param db 数据库连接
 * @param pstmt 预处理 sql 语句
 * @param index 当前要绑定的数据的索引
 * @param userp 给回调函数的自定义参数
 */
typedef void (*stmt_batch_bind_cb)(sqlite3*, sqlite3_stmt*, int32_t, void*);

typedef void (*stmt_unmarshal_cb)(sqlite3_stmt*, void*);

/**
 * @brief 插入操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param cb 给sql语句绑定参数的回调函数
 * @param userp 给回调函数的自定义参数
 * @return 变更的行数
 */
int32_t dao_template_create(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp);

/**
 * @brief 插入操作的模板函数(主键回填/主键回显)
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param cb 给sql语句绑定参数的回调函数
 * @param userp 给回调函数的自定义参数
 * @param pk 主键
 * @return 变更的行数
 */
int32_t dao_template_create_pk_backfill(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp, int32_t *pk);

/**
 * @brief 删除操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param cb 给sql语句绑定参数的回调函数
 * @param userp 给回调函数的自定义参数
 * @return 变更的行数
 */
int32_t dao_template_delete(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp);

/**
 * @brief 更新操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param cb 给sql语句绑定参数的回调函数
 * @param userp 给回调函数的自定义参数
 * @return 变更的行数
 */
int32_t dao_template_update(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp);

/**
 * @brief 删表操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @return true：成功；false：失败
 */
bool dao_template_drop(prepared_stmt_t *ps, const char *sql);

/**
 * @brief 查询操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param bind_cb 给sql语句绑定参数的回调函数
 * @param bp 给绑定参数的回调函数的自定义参数
 * @param unmarshal_cb 从stmt中取数据的回调函数
 * @param up 给stmt中取数据的回调函数的自定义参数
 * @return 查询到的行数
 */
int32_t dao_template_get(prepared_stmt_t *ps, const char *sql, stmt_bind_cb bind_cb, void *bp, stmt_unmarshal_cb unmarshal_cb, void *up);

/**
 * @brief 批量插入操作的模板函数
 * @param ps prepared_stmt_t 对象
 * @param sql sql 字符串
 * @param batch_size 批处理大小
 * @param cb 给 sql 语句绑定参数的回调函数
 * @param userp 给回调函数的自定义参数
 * @return 变更的行数
 */
int32_t dao_template_batch_create(prepared_stmt_t *ps, const char *sql, int32_t batch_size, stmt_batch_bind_cb cb, void *userp);

/**
 * @brief 具有可选项的查询操作的模板函数
 * @param ps prepared_stmt_t对象
 * @param sql sql字符串
 * @param options 可选项列表
 * @param unmarshal_cb 从stmt中取数据的回调函数
 * @param up 给stmt中取数据的回调函数的自定义参数
 * @return 查询到的行数
 */
int32_t dao_template_get_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options, stmt_unmarshal_cb unmarshal_cb, void *up);

/**
 * @brief 具有可选项的删除操作的模板函数
 * @param ps prepared_stmt_t 对象
 * @param sql sql 字符串
 * @param options 可选项列表
 * @return 变更的行数
 */
int32_t dao_template_delete_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options);

/**
 * @brief 具有可选项的更新操作的模板函数
 * @param ps prepared_stmt_t 对象
 * @param sql sql 字符串
 * @param options 可选项列表
 * @return 变更的行数
 */
int32_t dao_template_update_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options);

/**
 * @brief 开启事务
 */
void dao_begin(sqlite3 *db, const char *func_name);

/**
 * @brief 回滚事务
 */
void dao_rollback(sqlite3 *db, const char *func_name);

/**
 * @brief 提交事务
 */
void dao_commit(sqlite3 *db, const char *func_name);

/**
 * @brief 关闭数据库连接并删除该连接所有相关的stmt
 */
void dao_close(sqlite3 *db);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPLATE_H__ */
