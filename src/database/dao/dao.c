#include "dao.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#include <stdbool.h>
#include <stdlib.h>

static void init_relay(relay_dao_t *p)
{
    p->key = NULL;
}

static void copy_relay(relay_dao_t *dst, const relay_dao_t *src)
{
    dst->cnt_value = src->cnt_value;
    if (src->key)
    {
        dst->key = strdup(src->key);
    }
    else
    {
        dst->key = NULL;
    }
}

static void dtor_relay(relay_dao_t *p)
{
    free(p->key);
}

void init_dao(dao_t *dao)
{
    init_relay_dao(&dao->pcu_relay_cnt_dao);
    dao->relay_ut_icd = (UT_icd)
    {
        .sz = sizeof(relay_dao_t),
        (init_f*)init_relay,
        (ctor_f*)copy_relay,
        (dtor_f*)dtor_relay
    };
}
