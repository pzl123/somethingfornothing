#pragma once

#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    UT_icd relay_ut_icd;
    relay_dao_t *pcu_relay_cnt_dao;
} dao_t;

void init_dao(dao_t *dao);

#ifdef __cplusplus
}
#endif /* __cplusplus */
