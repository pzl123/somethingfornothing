#include <iostream>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

int main(void)
{
    dao_t *dao = dao_init();

    pcu_relay_cnt_t p0 = {DO_DC_INPUT1_POS, 20};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p0);
    pcu_relay_cnt_t p1 = {DO_DC_INPUT1_NEG, 21};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p1);
    pcu_relay_cnt_t p2 = {DO_DC_INPUT1_PRE, 22};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p2);
    pcu_relay_cnt_t p3 = {DO_DC_INPUT2_POS, 23};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p3);
    pcu_relay_cnt_t p4 = {DO_DC_INPUT2_NEG, 24};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p4);
    pcu_relay_cnt_t p5 = {DO_DC_INPUT2_PRE, 25};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p5);


    pcu_do_branch_relay_e relay_id = DO_DC_INPUT2_PRE;
    pcu_relay_cnt_t res;
    pcu_relay_cnt_dao_get_by_relay_id(&dao->pcu_relay_cnt_dao, relay_id, &res);
    d_log("DO_DC_INPUT2_PRE cnt:%d", res.close_cnt);

    pcu_relay_cnt_dao_delete_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_POS);
    pcu_relay_cnt_t p = {DO_DC_INPUT1_NEG, 200};
    pcu_relay_cnt_dao_update_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_NEG, &p);
    dao_clear();

    return 0;
}