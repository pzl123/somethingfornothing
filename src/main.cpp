#include <iostream>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"


int main(void)
{
    log_init();
    dao_t *dao = dao_init();
    pcu_relay_cnt_t *relay_cnt = get_relay_cnt();
    for (uint8_t i = 0; i < 6; i++)
    {
        pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &relay_cnt[i]);
    }


    pcu_do_branch_relay_e relay_id = DO_DC_INPUT2_PRE;
    pcu_relay_cnt_t res;
    pcu_relay_cnt_dao_get_by_relay_id(&dao->pcu_relay_cnt_dao, relay_id, &res);
    d_log("DO_DC_INPUT2_PRE cnt:%d", res.close_cnt);

    pcu_relay_cnt_dao_delete_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_POS);
    pcu_relay_cnt_t p = {7, DO_DC_INPUT1_NEG, 212};
    pcu_relay_cnt_dao_update_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_NEG, &p);

    fcgi_main(dao);

    dao_clear();
    return 0;
}