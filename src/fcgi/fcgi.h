#ifndef __FCGI_H__
#define __FCGI_H__

#include "database/dao/dao.h"

#ifdef __cplusplus
extern "C" {
#endif

void fcgi_main(dao_t *dao);

#ifdef __cplusplus
}
#endif

#endif /* __FCGI_H__ */
