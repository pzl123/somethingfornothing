#pragma once

#include <stdio.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>

typedef struct
{
    int aaa;
}dal_t;


/**
 * @brief create a db and some server
 *
 * @return bool
 */
bool new_dataserver(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
