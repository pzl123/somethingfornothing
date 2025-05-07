#pragma once

#include "uthash/utarray.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 基于 uthash 的 LRU 缓存实现 */

typedef struct
{
    UT_icd icd;
    void *data;
} cache_item_t;

void hash_add_or_update(relay_dao_t **user, const UT_icd *icd, const char *key, int value);
int32_t hash_find(relay_dao_t **user, const char *key);
void hash_delete(relay_dao_t **user, const UT_icd *icd, const char *key);
void hash_print_all(relay_dao_t **user);
void hash_clear_all(relay_dao_t **user,  const UT_icd *icd);

#ifdef __cplusplus
}
#endif /* __cplusplus */
