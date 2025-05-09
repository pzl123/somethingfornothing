#ifndef __DAO_OPTION_H__
#define __DAO_OPTION_H__



#include <stdint.h>
#include "sqlite3/sqlite3.h"
#include "uthash/utstring.h"
#include "uthash/utarray.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    DAO_TYPE_INT32,
    DAO_TYPE_INT64,
    DAO_TYPE_FLOAT32,
    DAO_TYPE_FLOAT64,
    DAO_TYPE_TEXT,
    DAO_TYPE_NULL,
}dao_type_e;

typedef struct
{
    void *userp;
    const char *sql_part;
    dao_type_e type;
}dao_option_t;

UT_array* new_dao_option_array(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DAO_OPTION_H__ */
