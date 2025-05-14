#ifndef __SQLITE_MASTER_DAO_H__
#define __SQLITE_MASTER_DAO_H__

#include "database/dao/prepared_stmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sqlite_master表是系统表，存储了数据库的元数据信息，包括所有表、索引、视图等的定义 */
typedef struct
{
    prepared_stmt_t *ps;
} sqlite_master_dao_t;

bool new_sqlite_master_dao(sqlite_master_dao_t *dao, prepared_stmt_t *ps);

void delete_sqlite_master_dao(sqlite_master_dao_t *dao);

/**
 * @brief 传入表名，求这张表在sqlite_master中有多少条记录
 * @param dao
 * @param name 动态码
 * @param count 返回结果
 * @return 查询到的行数
 */
int32_t sqlite_master_dao_count_by_name(sqlite_master_dao_t *dao, const char *name, int32_t *count);

#ifdef __cplusplus
}
#endif

#endif /* __SQLITE_MASTER_DAO_H__ */
