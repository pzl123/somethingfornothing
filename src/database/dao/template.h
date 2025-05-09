#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include "utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAO_RETURN_IS_OK(func)                      \
if (SQLITE_OK != func)                              \
{                                                   \
    d_log("sql_err=%s", sqlite3_errmsg(db));  \
}

#ifdef __cplusplus
}
#endif

#endif /* __TEMPLATE_H__ */
