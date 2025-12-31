#define _GNU_SOURCE

#include "can.h"
#include "utils/utils.h"
#include "module_queue_manager.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <string.h>
#define KERNAL_TASK_COMM_LEN 16 /* 内核线程名长度 */
#define DCDC_SET_QUEUE_SIZE 20U
#define DCDC_SET_THREAD_NUM  1U
#define F_TO_U_FACTOR 1000
#define DCDC_DATA_LEN 4
#define DCDC_CUR_GAIN 1024

typedef union
{
    uint32_t int_val;
    float float_val;
} value_data_u;

static can_task_pool_t g_dcdc_set_pool = {0};
module_queue_mgr_t *mgr;

int32_t taskpool_init(can_task_pool_t *pool, void *(task_func)(void *), int32_t queue_size, int32_t thread_num, uint64_t fr_interval_time, const char *pth_name)
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
        if (pthread_create(&pool->tid[i], NULL, task_func, (void *)pool) != 0)
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

int32_t dcdc_send_encapsulation(struct can_frame frame)
{
    // int32_t can_fd = 0;
    // get_dcdc_g_dcdc_can_fd(&can_fd);
    // if (!can_send_frame(can_fd, &frame))
    // {
    //     return PCU_ERR;
    // }
    i_log("dcdc send frame.can_id:%08x, frame.data: %02x %02x %02x %02x %02x %02x %02x %02x",frame.can_id,
                frame.data[0], frame.data[1], frame.data[2], frame.data[3],
                frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
    return PCU_ERR_SUCCESS;
}

static void int32_2_can_need_format(uint8_t *data, uint32_t value)
{
    uint32_t network_order = htonl(value);
    (void)memcpy(data, &network_order, sizeof(network_order));
}

static void float32_2_can_need_format(uint8_t *data, float value)
{
    convert_byte_order(&value, sizeof(uint32_t), true);
    (void)memcpy(data, &value, DCDC_DATA_LEN);
}

pq_t *get_g_dcdc_set_pool_queue_ptr(void)
{
    return g_dcdc_set_pool.queue;
}


bool submit_task(pq_t *queue, struct can_frame frame, int32_t (*callback)(struct can_frame frame), int32_t priority)
{
    if (queue == NULL /* || callback == NULL */)
    {
        if (callback == &dcdc_send_encapsulation)
        {
            e_log("dcdc queue is NULL in %s", __FUNCTION__);
        }
        else
        {
            e_log("ignore");
        }
        return false;
    }
    can_msg_t *task = (can_msg_t *)malloc(sizeof(can_msg_t));
    task->frame = frame;
    task->callback = callback;
    pv_t item = {0};
    item._value = (void *)task;
    item._priority = priority;
    item.timestamp = gettime_msec();
    // i_log("Submit task priority=%d queue:capacity:%d, size:%d utils:%f", priority, queue->capacity, queue->size, (float)(((float)(queue->size))/((float)(queue->capacity))));
    d_log("push in tasktread_queue");
    if (false == priority_queue_push(queue, item))
    {
        free(item._value);
        return false;
    }
    return true;
}

dcdc_can_id_u can_id_fill_dst(dc_model_param_t can_id_param)
{
    uint8_t dst = can_id_param.dst;
    uint8_t group = can_id_param.group;
    uint8_t ptp = can_id_param.ptp;

    dcdc_can_id_u can_id = {0};
    can_id.can_id_info.ptp = ptp;
    if (1 == ptp) /* 点对点 */
    {
        can_id.can_id_info.dst_addr = dst;
    }
    else if (0 == ptp) /* 广播 */
    {
        if (DST_EXT_BROADCAST == dst) /* 全局广播 */
        {
            can_id.can_id_info.dst_addr = dst;
        }
        else if (DST_BROADCAST == dst) /* 7组内广播 */
        {
            can_id.can_id_info.dst_addr = dst;
            can_id.can_id_info.group = group;
        }
        else/* 拓展组广播 */
        {
            can_id.can_id_info.dst_addr = (uint8_t)(0xFDU - dst);
        }
    }
    else
    {
        e_log("ptp value error, input 1 or 0");
        can_id.id = ERROR_ID;
        return can_id;
    }
    can_id.can_id_info.src_addr = SRC_ADDR;
    can_id.can_id_info.protno = PROTNO;
    return can_id;
}

static void set_frame_fill(struct can_frame *frame, uint16_t register_num, uint8_t *data, dc_model_param_t can_id_param)
{
    dcdc_can_id_u can_id = {0};
    can_id = can_id_fill_dst(can_id_param);
    (void)memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = can_id.id | CAN_EFF_FLAG;
    frame->can_dlc = CAN_MAX_DLC;
    frame->data[0] = DCDC_CAN_FUNC_CODE_SET;
    /* frame->data[1] = 0x00; */
    frame->data[2] = (uint8_t)(register_num >> 8);
    frame->data[3] = (uint8_t)(register_num);
    (void)memcpy(&frame->data[4], data, DCDC_DATA_LEN);
}

static inline bool generic_set_mode_function(dc_model_param_t can_id_param, value_data_u value, uint32_t cmd_type, const char *str, bool is_float, int32_t priority)
{
    (void)str;
    uint8_t data[DCDC_DATA_LEN] = {0};
    if (is_float)
    {
        float32_2_can_need_format(data, value.float_val);
        // LOG_DST_INFO_F(can_id_param, str, value.float_val);
    }
    else
    {
        int32_2_can_need_format(data, value.int_val);
        // LOG_DST_INFO_D(can_id_param, str, value.int_val);
    }
    can_msg_t *taskdata = (can_msg_t *)malloc(sizeof(can_msg_t));
    set_frame_fill(&taskdata->frame, cmd_type, data, can_id_param);
    dcdc_task_t task =
    {
        .priority = NORMAL,
        .data = taskdata,
        .task_id = GET_MODE_OUTPUT_VOL,
        .module_id = 5
    };
    d_log("push in module_queue[%d]: task_id[%d]", task.module_id, task.task_id);
    module_push_task(mgr, task.module_id, task);
    // return submit_task(get_g_dcdc_set_pool_queue_ptr(), frame, &dcdc_send_encapsulation, priority);
}


bool set_mode_out_cur(dc_model_param_t can_id_param, float out_cur)
{
    return generic_set_mode_function(can_id_param,
                                     (value_data_u){.int_val = (uint32_t)(out_cur * DCDC_CUR_GAIN)},
                                     SET_MODE_OUTPUT_CUR,
                                     __func__,
                                     false,
                                     NORMAL);
}


static void *task_work(void *arg)
{
    if (NULL == arg)
    {
        e_log("arg is NULL");
        return NULL;
    }
    can_task_pool_t *pool = (can_task_pool_t *)arg;
    pv_t item = {0};

    while (1)
    {
        if (true == priority_queue_pop(pool->queue, &item)) /* 出线程池队列 */
        {
            can_msg_t *task = (can_msg_t *)item._value;
            if (task->callback == NULL)
            {
                i_log("Thread %lu exiting...", pthread_self());
                if (NULL != task)
                {
                    free(task);
                }
                return NULL;
            }
            d_log("pop from taskthread_queue");
            task->callback(task->frame);
            if (NULL != task)
            {
                free(task);
            }

            if (0 != pool->fr_interval_time)
            {
                usleep(pool->fr_interval_time);
            }

        }
    }
    return NULL;
}


void *module_queue_task_thread(void *arg)
{
    (void)prctl(PR_SET_NAME, __FUNCTION__);
    while (1)
    {
        dc_model_param_t can_id_param;
        can_id_param.dst = 5;
        can_id_param.group = 0;
        can_id_param.ptp = 1;
        float out_cur = 100.0f;
        set_mode_out_cur(can_id_param, out_cur); /* 入队 module_queue */
        dcdc_task_t *received = NULL;
        for (uint8_t i = 0U; i < 12; i++)
        {
            received = module_try_pop_task(mgr, i + 1); /* 出队 module_queue */
            if (NULL != received)
            {
                d_log("pop in module_queue[%d]: task_id[%d]", received->module_id, received->task_id);
                can_msg_t *taskptr = (can_msg_t *)received->data;
                if (!submit_task(get_g_dcdc_set_pool_queue_ptr(), taskptr->frame, &dcdc_send_encapsulation, received->priority)) /* 入线程池队列 */
                {
                    e_log("submit error, module[%d] task_id:%d", received->module_id, received->task_id);
                }
            }
            usleep(10 * 1000);
        }
    }
}

void test_dcdc_queue(void)
{
    mgr = module_queue_mgr_init();
    if (!mgr)
    {
        e_log("Failed to init module queue manager!");
        return;
    }
    (void)taskpool_init(&g_dcdc_set_pool, task_work, DCDC_SET_QUEUE_SIZE, DCDC_SET_THREAD_NUM, 20 * 1000, "dc_set");


    pthread_t module_queue_task_thread_id;
    pthread_create(&module_queue_task_thread_id, NULL, &module_queue_task_thread, NULL);
    pthread_join(module_queue_task_thread_id, NULL);
    module_queue_mgr_destroy(mgr);
    return;
}

void can_test(void)
{
    test_dcdc_queue();
    while(1)
    {
        sleep(1);
    }
}