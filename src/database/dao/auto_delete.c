#include "auto_delete.h"
#include "database/dao/retention_policy.h"
#include "utils/utils.h"

/**
 * @brief 定时器回调函数，遍历保留策略数组并执行每个保留策略
 * @param timer 定时器指针
 */
static void on_timer(htimer_t *timer)
{
    dao_deleter_t *d = (dao_deleter_t*)hevent_userdata(timer);  // 获取用户数据
    retention_policy_t *p = (retention_policy_t*)utarray_front(d->rps);
    int32_t i = 0;
    for (; p != NULL; p = (retention_policy_t*)utarray_next(d->rps, p))
    {
        bool ret = p->policy_func(p);
        if (false == ret)
        {
            d_log("run error i=%d", i);
        }
        i++;
    }
    return;
}

bool new_dao_deleter(dao_deleter_t *d, hloop_t *loop, UT_array *rps, uint32_t timeout_ms)
{
    if ((NULL == d)
    || (NULL == loop)
    || (NULL == rps))
    {
        d_log("api misuse");
        return false;
    }

    d->loop = loop;
    d->rps = rps;
    htimer_t *timer = htimer_add(d->loop, on_timer, timeout_ms, INFINITE);
    hevent_set_userdata(timer, d);  // 绑定用户数据
    return true;
}


void delete_dao_deleter(dao_deleter_t *d)
{
    if (NULL == d)
    {
        d_log("api misuse");
        return;
    }

    if (NULL != d->rps)
    {
        utarray_free(d->rps);
        d->rps = NULL;
    }
    d->loop = NULL;
    return;
}

UT_array* new_retention_policy_array(dao_t *dao)
{
    static const UT_icd icd = {
        .sz = sizeof(retention_policy_t),
        .init = NULL,
        .copy = NULL,
        .dtor = NULL,
    };

    /* 创建保留策略数组 */
    UT_array *rps = NULL;
    utarray_new(rps, &icd);
    /* 将各个保留策略添加到数组中 */
    utarray_push_back(rps, &dao->pcu_relay_cnt_dao.rp);

    return rps;
}
