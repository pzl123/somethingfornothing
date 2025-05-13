#ifndef __FILENAME_H__
#define __FILENAME_H__

#include "uthash/utarray.h"
#include "hv/hloop.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DAO_DELETER_TIMEOUT_10MIN (1000 * 60 * 10)

typedef struct
{
    UT_array *rps;
    hloop_t *loop;
} dao_deleter_t;

#ifdef __cplusplus
}
#endif

#endif /* __FILENAME_H__ */
