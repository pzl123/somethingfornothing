#ifndef __DAL_H__
#define __DAL_H__






#include <stdbool.h>

#include "database/dao/dao.h"
#include "database/dao/dsn.h"
#include "database/dao/prepared_stmt.h"
#include "database/dao/sqlite_master/sqlite_master_dao.h"
#include "database/dao/auto_delete.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    prepared_stmt_t pstmts[DSN_END];
    sqlite_master_dao_t sqlite_master_dao_array[DSN_END];
    dao_t dao;
    dao_deleter_t deleter;
    hloop_t *loop;
} dal_t;

bool new_dal(dal_t *dal);

void delete_dal(dal_t *dal);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DAL_H__*/
