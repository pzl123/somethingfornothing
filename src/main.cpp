#include <iostream>
#include "uthash/uthash.h"
#include "database/sqlite3_base.h"
#include "uthash/utarray.h"
#include "database/cache/lru.h"
#include "database/dal.h"


int main(void)
{
    dal_t dal;
    init_dal(&dal);
    hash_add_or_update(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd, "A", 100);
    hash_add_or_update(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd, "B", 200);
    hash_add_or_update(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd, "C", 300);

    hash_print_all(&dal.dao.pcu_relay_cnt_dao);

    hash_add_or_update(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd, "A", 999);
    hash_print_all(&dal.dao.pcu_relay_cnt_dao);


    hash_find(&dal.dao.pcu_relay_cnt_dao, "B");
    hash_find(&dal.dao.pcu_relay_cnt_dao, "X");


    hash_delete(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd, "B");
    hash_print_all(&dal.dao.pcu_relay_cnt_dao);


    hash_clear_all(&dal.dao.pcu_relay_cnt_dao, &dal.dao.relay_ut_icd);
    hash_print_all(&dal.dao.pcu_relay_cnt_dao);

    new_dataserver();
    return 0;
}