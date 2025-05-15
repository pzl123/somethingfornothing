#ifndef __RELAY_CNT_H__
#define __RELAY_CNT_H__

#include "fcgi/fcgi_stdio.h"
#include "fcgi/errors/api_code.h"

#ifdef __cplusplus
extern "C" {
#endif
void set_relay_cnt(FCGX_Request *req);
void query_relay_cnt(FCGX_Request *req);
#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CNT_H__ */
