#pragma once

#include "database/dao/dao.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    dao_t dao;
} dal_t;

void init_dal(dal_t *dal);

#ifdef __cplusplus
}
#endif /* __cplusplus */
