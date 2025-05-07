#pragma once

#include "uthash/uthash.h"
#include "uthash/utarray.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    char *key;
    int32_t cnt_value;
    UT_hash_handle hh;
} relay_dao_t;

void init_relay_dao(relay_dao_t **hash_head);

#ifdef __cplusplus
}
#endif /* __cplusplus */
