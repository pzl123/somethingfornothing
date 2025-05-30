#define _GNU_SOURCE

#include "can.h"
#include "utils/utils.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define KERNAL_TASK_COMM_LEN 16 /* 内核线程名长度 */


static void *task_work(void *arg)
{
    if (NULL == arg)
    {
        e_log("arg is NULL");
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
        pq_delete(pool->queue);
        free(pool->queue);
        return -1;
    }
    pool->thread_count = thread_num;
    for (uint8_t i = 0; i < thread_num; i++)
    {
        if (pthread_create(&pool->tid[i], NULL, task_work, (void *)pool) != 0)
        {
            e_log("pthread_create failed in %s", __FUNCTION__);
            for (uint8_t j = 0; j < i; j++)
            {
                pthread_cancel(pool->tid[j]);
                pthread_join(pool->tid[j], NULL);
            }
            free(pool->tid);
            pq_delete(pool->queue);
            free(pool->queue);
            return -1;
        }
        char thread_name[KERNAL_TASK_COMM_LEN] = {0};
        (void)snprintf(thread_name, KERNAL_TASK_COMM_LEN, "%s_%d", pth_name, i);
        (void)pthread_setname_np(pool->tid[i], thread_name);
    }
    return 0;
}


int32_t can_func(struct can_frame frame)
{
    d_log("frame.id:[%08x] [%02x %02x %02x %02x %02x %02x %02x %02x ]", frame.can_id,\
    frame.data[0], frame.data[1], frame.data[2], frame.data[3],\
    frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
    return 0;
}

void *func1(void *arg)
{
    pq_t *pq = (pq_t *)arg;
    struct can_frame frame =
    {
        .can_id = 0x060F8039,
        .can_dlc = 8,
        .data = {0x41, 0xF0, 0x00, 0x01, 0x3E, 0xC8, 0x00, 0x00}
    };
    can_msg_t msg  = {.frame = frame, .callback = can_func};

    pv_t item1 = {._priority = 11, ._value = (void *)&msg};
    pv_t item2 = {._priority = 10, ._value = NULL};
    pv_t item3 = {._priority = 9, ._value = NULL};
    pv_t item4 = {._priority = 8, ._value = NULL};
    pv_t item5 = {._priority = 7, ._value = NULL};
    pv_t item6 = {._priority = 6, ._value = NULL};
    pv_t item7 = {._priority = 5, ._value = NULL};
    pv_t item8 = {._priority = 4, ._value = NULL};
    pv_t item9 = {._priority = 3, ._value = NULL};
    pv_t item10 = {._priority = 2, ._value = NULL};
    pv_t item11 = {._priority = 1, ._value = NULL};

    while (1)
    {
        priority_queue_push(pq, item1);
        usleep(1000);
        priority_queue_push(pq, item2);
        usleep(1000);
        priority_queue_push(pq, item3);
        usleep(1000);
        priority_queue_push(pq, item4);
        usleep(1000);
        priority_queue_push(pq, item5);
        usleep(1000);
        priority_queue_push(pq, item6);
        usleep(1000);
        priority_queue_push(pq, item7);
        usleep(1000);
        priority_queue_push(pq, item8);
        usleep(1000);
        priority_queue_push(pq, item9);
        usleep(1000);
        priority_queue_push(pq, item10);
        usleep(1000);
        priority_queue_push(pq, item11);
        usleep(1000);
    }
    return NULL;
}

void *func2(void *arg)
{
    pq_t *pq = (pq_t *)arg;

    while (1)
    {
        // pv_t item = pq_top(pq);
        pv_t item = {0};
        priority_queue_pop(pq, &item);
        if (item._value != NULL)
        {
            can_msg_t *tmp = (can_msg_t *)item._value;
            if (NULL != tmp->callback)
            {
                d_log("Top key: %d", item._priority);
                tmp->callback(tmp->frame);
            }
            else
            {
                d_log("callback is NULL");
            }

            // if (item._icd._dele)
            // {
            //     item._icd._dele(item._value);
            // }
            // else
            // {
            //     d_log("icd._dele is NULL");
            // }
        }
        else
        {
            d_log("Top key: %d _value is NULL", item._priority);
        }

        usleep(1000);
    }
    return NULL;
}

void test(void)
{
    pq_t *pq = pq_init(10, max_heap_compare);


    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, func1, (void *)pq);
    pthread_create(&tid2, NULL, func2, (void *)pq);

    while(1)
    {
        sleep(1);
    }
}