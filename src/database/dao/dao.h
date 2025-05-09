#ifndef __DAO_H__
#define __DAO_H__





#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    pcu_relay_cnt_dao_t *pcu_relay_cnt_dao;
} dao_t;

void init_dao(dao_t *dao);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DAO_H__ */
