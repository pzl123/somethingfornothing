#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define DAO_RETURN_IS_OK(func)                      \
if (SQLITE_OK != func)                              \
{                                                   \
    printf("sql_err=%s", sqlite3_errmsg(db));  \
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
