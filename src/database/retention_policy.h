#ifndef __RETENTION_H__
#define __RETENTION_H__


#include <stdbool.h>

#include "database/dao/prepared_stmt.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RETENTION_POLICY_PERCENT_90     (90)
#define RETENTION_POLICY_EXPIRE_90DAY   (60 * 60 * 24 * 90)
#define RETENTION_POLICY_EXPIRE_180DAY  (60 * 60 * 24 * 180)
#define RETENTION_POLICY_1000_LINE      (1000)

typedef struct retention_policy_t retention_policy_t;
struct retention_policy_t
{
    void *userp;
    bool (*run)(retention_policy_t*);
};

/**
 * @brief 保留策略(通过行数删除记录)
 * @param ps 预处理语句指针
 * @param table_name 表名
 * @param max_num 最大行数
 * @param percent 保留百分比(0-100)。达到最大行数时，删除记录直到总数为最大行数的百分之 ？
 */
bool new_retention_policy_by_line(retention_policy_t *r, prepared_stmt_t *ps, const char *table_name, int32_t max_num, int32_t percent);

void delete_retention_policy_by_line(retention_policy_t *r);

/**
 * @brief 保留策略(通过时间戳删除记录)
 * @param ps 预处理语句指针
 * @param table_name 表名
 * @param where_field 时间戳字段名
 * @param expire_s 过期时间，单位秒
 */
bool new_retention_policy_by_timestamp(retention_policy_t *r, prepared_stmt_t *ps, const char *table_name, const char *where_field, int64_t expire_s);

void delete_retention_policy_by_timestamp(retention_policy_t *r);

/**
 * @brief 保留策略(永久)
 */
bool new_retention_policy_always(retention_policy_t *r);

void delete_retention_policy_always(retention_policy_t *r);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __RETENTION_H__ */
