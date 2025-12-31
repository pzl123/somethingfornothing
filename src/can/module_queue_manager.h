// module_queue_manager.h
#ifndef __MODULE_QUEUE_MANAGER_H__
#define __MODULE_QUEUE_MANAGER_H__

#include "utils/priority_queue/priority_queue.h"
#include "can.h"


typedef struct
{
    pq_t *queues[MODULE_COUNT];  // 每个模块一个队列
} module_queue_mgr_t;

// 初始化所有队列（使用 aging_compare 实现老化）
module_queue_mgr_t* module_queue_mgr_init(void);

// 销毁所有队列
void module_queue_mgr_destroy(module_queue_mgr_t *mgr);

// 向指定模块推送任务
bool module_push_task(module_queue_mgr_t *mgr, int module_id, const dcdc_task_t task);

// 从指定模块阻塞弹出任务
dcdc_task_t *module_pop_task(module_queue_mgr_t *mgr, int module_id);

// 从指定模块非阻塞弹出任务
dcdc_task_t *module_try_pop_task(module_queue_mgr_t *mgr, int module_id);


#endif