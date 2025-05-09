#ifndef __DAL_H__
#define __DAL_H__






#include <stdbool.h>

#include "database/dao/dao.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    // prepared_stmt_t pstmts[DSN_END];
    // sqlite_master_dao_t sqlite_master_dao_array[DSN_END];
    dao_t dao;
} dal_t;

void init_dal(dal_t *dal);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DAL_H__*/
