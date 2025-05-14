#ifndef __TABLE_H__
#define __TABLE_H__





#include <stdint.h>
#include <stdbool.h>

#include "database/dao/dsn.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    const char *table_name;                 /* 表名 */
    db_dsn_e dsn;                           /* 数据源名称，包含了连接到数据库所需的所有必要信息 */
    const char *create_table_stmt;          /* 建表语句 */
}table_t;

bool get_tables(const table_t **head, int32_t *len);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __TABLE_H__ */
