#include <iostream>

#include "utils/cache/lru.h"
#include "database/init.h"

int main(void)
{
    dao_t *dao = dao_init();
    pcu_relay_cnt_t p = {1, DO_DC_INPUT1_POS, 22};
    pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &p);
    return 0;
}