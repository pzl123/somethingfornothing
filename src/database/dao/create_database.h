#ifndef __CREATE_DATABASHE_H__
#define __CREATE_DATABASHE_H__

#include "database/dao/sqlite_master/sqlite_master_dao.h"
#include "database/model/table.h"

#ifdef __cplusplus
extern "C" {
#endif

bool create_table(sqlite_master_dao_t *dao, const table_t *table);

/**
 * @brief 创建数据库并初始化里面表，对于已经存在的表不会进行修改
 * @param dao 系统表数据访问对象(判断表是否存在需要访问系统表)
 * @param tables 待创建的表的数组
 * @param len 数组长度
 */
bool create_database(sqlite_master_dao_t *dao, const table_t *tables, int32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __CREATE_DATABASHE_H__ */
