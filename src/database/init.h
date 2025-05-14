#ifndef __DAL_INIT_H__
#define __DAL_INIT_H__

#include "database/dao/dao.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

dao_t* dao_init(void);

void dao_clear(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif