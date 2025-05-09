#include "dao.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#include <stdbool.h>
#include <stdlib.h>

bool new_dao(dao_t *dao, prepared_stmt_t *pstmts,/*  hloop_t *loop, */ sqlite_master_dao_t *sqlite_master_dao_array)
{
    if ((NULL == dao)
    || (NULL == pstmts)
    /* || (NULL == loop) */)
    {
        d_log("api misuse");
        return false;
    }
    (void)sqlite_master_dao_array;

    (void)new_pcu_relay_cnt_dao(&dao->pcu_relay_cnt_dao, &pstmts[DSN_MAIN]);

    return true;
}

void delete_dao(dao_t *dao)
{
    if (NULL == dao)
    {
        d_log("api misuse");
        return;
    }

    delete_pcu_relay_cnt_dao(&dao->pcu_relay_cnt_dao);

    return;
}