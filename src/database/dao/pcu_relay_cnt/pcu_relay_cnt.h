#ifndef __PCURELAY_CNT_H__
#define __PCURELAY_CNT_H__




#include "uthash/uthash.h"
#include "uthash/utarray.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    int32_t aaa;
    // retention_policy_t rp;
    // prepared_stmt_t *ps;
} pcu_relay_cnt_dao_t;

void init_relay_dao(pcu_relay_cnt_dao_t **hash_head);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __PCURELAY_CNT_H__ */
