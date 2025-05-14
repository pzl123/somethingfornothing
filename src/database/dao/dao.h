#ifndef __DAO_H__
#define __DAO_H__




#include "hv/hloop.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "database/dao/sqlite_master/sqlite_master_dao.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    pcu_relay_cnt_dao_t pcu_relay_cnt_dao;
} dao_t;

bool new_dao(dao_t *dao, prepared_stmt_t *pstmts, hloop_t *loop, sqlite_master_dao_t *sqlite_master_dao_array);

void delete_dao(dao_t *dao);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DAO_H__ */
