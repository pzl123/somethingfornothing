#ifndef __AUTO_DELETE_H__
#define __AUTO_DELETE_H__

#include "uthash/utarray.h"
#include "hv/hloop.h"
#include "database/dao/dao.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAO_DELETER_TIMEOUT_10MIN (1000 * 60 * 10)

typedef struct
{
    UT_array *rps;
    hloop_t *loop;
} dao_deleter_t;

/**
 * @brief 创建 dao_deleter_t 对象
 * @param d dao_deleter_t 对象指针
 * @param loop 事件循环对象指针
 * @param rps 保留策略数组指针
 * @param timeout_ms 定时器触发间隔（毫秒）
 * @return 成功返回 true，失败返回 false
 */
bool new_dao_deleter(dao_deleter_t *d, hloop_t *loop, UT_array *rps, uint32_t timeout_ms);

/**
 * @brief 删除 dao_deleter_t 对象
 * @param d dao_deleter_t 对象指针
 */
void delete_dao_deleter(dao_deleter_t *d);

/**
 * @brief 创建保留策略数组
 * @param dao dao 对象指针
 * @return 保留策略数组指针
 */
UT_array* new_retention_policy_array(dao_t *dao);

#ifdef __cplusplus
}
#endif

#endif /* __AUTO_DELETE_H__ */
