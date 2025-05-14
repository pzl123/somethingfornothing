#include <iostream>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi_stdio.h"

int main(void)
{
    log_init();

    dao_t *dao = dao_init();

    pcu_relay_cnt_t p0 = {1, DO_DC_INPUT1_POS, 20};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p0);
    pcu_relay_cnt_t p1 = {2, DO_DC_INPUT1_NEG, 21};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p1);
    pcu_relay_cnt_t p2 = {3, DO_DC_INPUT1_PRE, 22};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p2);
    pcu_relay_cnt_t p3 = {4, DO_DC_INPUT2_POS, 23};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p3);
    pcu_relay_cnt_t p4 = {5, DO_DC_INPUT2_NEG, 24};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p4);
    pcu_relay_cnt_t p5 = {6, DO_DC_INPUT2_PRE, 25};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p5);


    pcu_do_branch_relay_e relay_id = DO_DC_INPUT2_PRE;
    pcu_relay_cnt_t res;
    pcu_relay_cnt_dao_get_by_relay_id(&dao->pcu_relay_cnt_dao, relay_id, &res);
    d_log("DO_DC_INPUT2_PRE cnt:%d", res.close_cnt);

    pcu_relay_cnt_dao_delete_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_POS);
    pcu_relay_cnt_t p = {7, DO_DC_INPUT1_NEG, 200};
    pcu_relay_cnt_dao_update_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_NEG, &p);
    dao_clear();

    FCGX_Init();
    FCGX_Request req;
    memset(&req, 0, sizeof(req));
    FCGX_InitRequest(&req, 0, 0);

    while (0 == FCGX_Accept_r(&req))
    {
        sleep(1);
        FCGX_FPrintF(req.out, "Content-Type: text/plain\r\n\r\n");
        FCGX_FPrintF(req.out, "Hello FastCGI World!\n");
        FCGX_Finish_r(&req);
        d_log("*************** one time  ***************");
    }

    return 0;
}