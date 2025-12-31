#include <stdlib.h>
#include <string.h>

#include "module_queue_manager.h"
#include "utils/utils.h"

#define DCDC_QUEUE_LEN (20)

static int32_t priority_to_int(task_priority_e p)
{
    return (int32_t)p;
}

module_queue_mgr_t *module_queue_mgr_init(void)
{
    module_queue_mgr_t *mgr = (module_queue_mgr_t *)malloc(sizeof(module_queue_mgr_t));
    if (!mgr)
    {
        return NULL;
    }

    for (int i = 0; i < MODULE_COUNT; i++)
    {
        // 使用 aging_compare 实现“老化”：长时间等待的任务优先级自动提升
        mgr->queues[i] = pq_init(DCDC_QUEUE_LEN, aging_compare);
        if (!mgr->queues[i])
        {
            for (int j = 0; j < i; j++)
            {
                pq_delete(mgr->queues[j]);
            }
            free(mgr);
            return NULL;
        }
        // d_log("queue[%d]:capacity:%u", i, mgr->queues[i]->capacity);
    }
    return mgr;
}

void module_queue_mgr_destroy(module_queue_mgr_t *mgr)
{
    if (!mgr)
    {
        e_log("mgr is null");
        return;
    }
    for (int i = 0; i < MODULE_COUNT; i++)
    {
        pq_delete(mgr->queues[i]);
    }
    free(mgr);
}

static void free_task(dcdc_task_t *taskptr)
{
    free(taskptr->data);
    free(taskptr);
}

bool module_push_task(module_queue_mgr_t *mgr, int module_id, const dcdc_task_t task)
{
    if (!mgr || module_id <= 0 || module_id > MODULE_COUNT)
    {
        e_log("api error");
        return false;
    }

    pq_t *pq = mgr->queues[module_id - 1];
    if (!pq)
    {
        e_log("Queue for module %d not initialized!", module_id);
        return false;
    }
    bool duplicate = false;

    pthread_mutex_lock(&pq->mutex);
    for (int i = 0; i < pq->size; i++)
    {
        dcdc_task_t *existing = (dcdc_task_t *)pq->elements[i]._value;
        if ((NULL != existing) && (existing->task_id == task.task_id) && (existing->module_id == task.module_id))
        {
            duplicate = true;
            break;
        }
    }
    pthread_mutex_unlock(&pq->mutex);

    if (true == duplicate)
    {
        return true;
    }
    else
    {
        if (SET_MODE_SWITCH == task.task_id)
        {
            priority_queue_clear(mgr->queues[module_id - 1], free_task);
        }

        dcdc_task_t *task_cp = (dcdc_task_t *)malloc(sizeof(dcdc_task_t));
        task_cp->priority = task.priority;
        task_cp->module_id = task.module_id;
        task_cp->task_id = task.task_id;
        can_msg_t *msg_copy = malloc(sizeof(can_msg_t));
        if (!msg_copy)
        {
            return false;
        }
        (void)memcpy(msg_copy, task.data, sizeof(can_msg_t));
        task_cp->data = (void *)msg_copy;
        pv_t item =
        {
            ._priority = priority_to_int(task_cp->priority),
            ._value = (void *)task_cp,
            .timestamp = gettime_msec()
        };

        return priority_queue_push(mgr->queues[module_id - 1], item);
    }
}


dcdc_task_t *module_pop_task(module_queue_mgr_t *mgr, int module_id)
{
    if (!mgr || module_id <= 0 || module_id > MODULE_COUNT)
    {
        e_log("api error");
        return NULL;
    }
    pv_t item;
    int32_t module_index = module_id - 1;
    if (!priority_queue_pop(mgr->queues[module_index], &item))
    {
        return NULL;
    }
    return (dcdc_task_t *)item._value;
}

dcdc_task_t *module_try_pop_task(module_queue_mgr_t *mgr, int module_id)
{
    if (!mgr || module_id <= 0 || module_id > MODULE_COUNT)
    {
        e_log("api error");
        return NULL;
    }
    pv_t item;
    int32_t module_index = module_id - 1;
    if (!priority_queue_try_pop(mgr->queues[module_index], &item))
    {
        return NULL;
    }
    return (dcdc_task_t *)item._value;
}