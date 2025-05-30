#include "can.h"
#include "utils/utils.h"

#define KERNAL_TASK_COMM_LEN 16 /* 内核线程名长度 */


static void *task_work(void *arg)
{
    if (NULL == arg)
    {
        log_e("arg is NULL");
        return NULL;
    }
    can_task_pool_t *pool = (can_task_pool_t *)arg;
    pv_t task = {0};
    while (1)
    {
        (void)priority_queue_pop(pool->queue, &task);
        can_msg_t *msg = (can_msg_t *)task._value;
        if (msg->callback == NULL)
        {
            // log_d("pdu_task_work thread %ld finished", pthread_self());
            break;
        }
        msg->callback(msg->frame);
        // log_i("frame id:%08x",task.frame.can_id);
    }
    return NULL;
}

int32_t taskpool_init(can_task_pool_t *pool, int32_t queue_size, int32_t thread_num, const char *pth_name)
{
    if (pool == NULL)
    {
        e_log("pool is NULL");
        return -1;
    }

    pool->queue = pq_init(queue_size, max_heap_compare);
    if (pool->queue == NULL)
    {
        e_log("malloc error");
        return -1;
    }

    pool->tid =(pthread_t *)malloc(sizeof(pthread_t) * thread_num);
    if (pool->tid == NULL)
    {
        e_log("malloc error in %s", __FUNCTION__);
        can_msg_queue_destroy(pool->queue);
        free(pool->queue);
        return -1;
    }
    pool->thread_count = thread_num;
    for (uint8_t i = 0; i < thread_num; i++)
    {
        if (pthread_create(&pool->tid[i], NULL, task_work, (void *)pool) != 0)
        {
            log_e("pthread_create failed in %s\n", __FUNCTION__);
            for (uint8_t j = 0; j < i; j++)
            {
                pthread_cancel(pool->tid[j]);
                pthread_join(pool->tid[j], NULL);
            }
            free(pool->tid);
            (void)can_msg_queue_destroy(pool->queue);
            free(pool->queue);
            return -1;
        }
        char thread_name[KERNAL_TASK_COMM_LEN] = {0};
        (void)snprintf(thread_name, KERNAL_TASK_COMM_LEN, "%s_%d", pth_name, i);
        (void)pthread_setname_np(pool->tid[i], thread_name);
    }
    return 0;
}

