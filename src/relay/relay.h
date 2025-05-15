#ifndef __RELAY_H__
#define __RELAY_H__

#include "database/model/pcu_relay_cnt_model.h"

#ifdef __cplusplus
extern "C" {
#endif

pcu_relay_cnt_t* get_relay_cnt(void);

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_H__ */
