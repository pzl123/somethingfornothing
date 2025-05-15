#ifndef __RELAY_CNT_H__
#define __RELAY_CNT_H__

#include "fcgi/fcgi_stdio.h"
#include "fcgi/errors/api_code.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int id;
    const char *name;
    const char *relay_id;
    int action_cnt;
} RelayData_t;

void query_relay_cnt(RelayData_t *relay_data, FCGX_Request *req);
#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CNT_H__ */
