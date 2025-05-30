#ifndef __CAN_H__
#define __CAN_H__

#include "utils/priority_queue/priority_queue.h"

#include <pthread.h>
#include <linux/can.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct can_msg
{
    struct can_frame frame;
    int32_t (*callback)(struct can_frame frame);//处理函数指针
} can_msg_t;

typedef struct can_task_pool
{
    pq_t *queue;
    pthread_t *tid;//工作线程池
    uint32_t thread_count; //工作线程数
} can_task_pool_t;

int32_t taskpool_init(can_task_pool_t *pool, int32_t queue_size, int32_t thread_num, const char *pth_name);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
