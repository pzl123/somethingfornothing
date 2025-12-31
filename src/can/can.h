#ifndef __CAN_H__
#define __CAN_H__

#include "utils/priority_queue/priority_queue.h"

#include <pthread.h>
#include <linux/can.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BROADCAST 0x0607FF80
#define ERROR_ID 0x00000000

#define PROTNO 0x060
#define SRC_ADDR 0xF0

#define DST_BROADCAST       0xFE
#define DST_EXT_BROADCAST   0xFF
#define PTP_BROADCAST       0
#define PTP_POINT           1

#define DCDC_FUNC_NO_COUNT (32)
#define MODULE_COUNT 12

typedef enum
{
    NONE = 0,
    NORMAL = 1,
    URGENT = 2,
    IMMEDIATE = 3
} task_priority_e;

typedef struct can_msg
{
    struct can_frame frame;
    int32_t (*callback)(struct can_frame frame);//处理函数指针
} can_msg_t;

typedef struct
{
    task_priority_e priority;   // 用于映射到 _priority
    can_msg_t *data;
    int32_t task_id;
    int32_t module_id;
} dcdc_task_t;

typedef enum
{
    GET_MODE_OUTPUT_VOL = 0x0001,                    /* 取模块输出电压 */
    GET_MODE_OUTPUT_CUR = 0x0002,                    /* 取模块输出电流 */
    GET_MODE_LIMIT_POIINT = 0x0003,                  /* 取模块限流点 */
    GET_MODE_DC_BORAD_TEMP = 0x0004,                 /* 取模块DC板温度 */
    GET_MODE_DC_INPUT_VOL = 0x0005,                  /* 取模块输入电压 */
    GET_MODE_PFC0_VOL = 0x0008,                      /* 取模块PFC0电压 正半母线 */
    GET_MODE_PFC1_VOL = 0x000A,                      /* 取模块PFC1电压 负半母线*/
    GET_MODE_ENV_TEMP = 0x000B,                      /* 取环境温度 */
    GET_MODE_PFC_temp = 0x0010,                      /* 取模块PFC板温度 */
    GET_MODE_RATED_OUTPUT_PWR = 0x0011,              /* 取模块额定输出功率 */
    GET_MODE_RATED_OUTPUT_CUR = 0x0012,              /* 取模块额定输出电流 */
    SET_MODE_WORK_ALTITUDE = 0x0017,                 /* 设置模块工作海拔 */
    SET_MODE_OUTPUT_CUR = 0x001B,                    /* 设置模块输出电流 */
    SET_MODE_GROUP_NUM = 0x001E,                     /* 设置模块组号 */
    SET_MODE_ADDR_ALLOC_METH = 0x001F,               /* 设置模块地址分配方式 */
    SET_MODE_OUTPUT_PWR = 0x0020,                    /* 设置模块输出功率 */
    SET_MODE_OUTPUT_VOL = 0x0021,                    /* 设置模块输出电压 */
    SET_MODE_LIMIT_POINT = 0x0022,                   /* 设置模块限流点 */
    SET_MODE_OUTPUT_VOL_MAX = 0x0023,                /* 设置模块最大输出电压 */
    SET_MODE_SWITCH = 0x0030,                        /* 设置模块开关机 */
    SET_MODE_OVER_VOL_RESET = 0x0031,                /* 设置模块过压复位 */
    SET_MODE_OUT_OVER_VOL_PROTECTION_RELATED = 0x3E, /* 设置模块过压保护关联 */
    GET_MODE_ALARM_BIT = 0x0040,                     /* 取模块告警状态 */
    GET_MODE_DIP_ADDR = 0x0043,                      /* 取模块拨码地址 */
    SET_MODE_SC_RESET = 0x0044,                      /* 设置模块短路复位 */
    SET_MODE_INPUT_MODEL = 0x0046,                   /* 设置模块输入模式 */
    GET_MODE_INPUT_PWR = 0x0048,                     /* 获取模块输入功率 */
    GET_MODE_WORK_ALTITUDE = 0x004A,                 /* 获取模块工作海拔 */
    GET_MODE_INPUT_MODEL = 0x004B,                   /* 获取模块输入模式 */
    GET_MODE_DCDC_SOFTWARE_VER = 0x0056,             /* 取模块DCDC版本号 */
    GET_MODE_PFC_SOFTWARE_VER = 0x0057,              /* 取模块PFC版本号 */
    SET_MODE_FAN = 0x0033                            /* 设置dcdc风扇 */
} dcdc_func_no_type;

typedef enum
{
    DCDC_CAN_FUNC_CODE_SET = 0x03,
    DCDC_CAN_FUNC_CODE_GET = 0x10,
} dcdc_can_func_code_e;

#pragma pack(push, 1)
typedef union
{
    uint32_t id;
    struct
    {
        uint8_t group : 3;
        uint8_t src_addr : 8;
        uint8_t dst_addr : 8;
        uint8_t ptp : 1;
        uint16_t protno : 9;
        uint8_t reserved : 3;
    } can_id_info;

} dcdc_can_id_u;
#pragma pack(pop)

typedef struct
{
    uint8_t ptp;   /* ptp = 1时, 点对点, ptp = 0时, 广播 */
    uint8_t dst;   /* 当ptp =1 时, 目的地址范围为00 ~ 63, 当ptp = 0 时，dst想要组内广播，填0xFE，拓展组广播填 组号 ,全局广播填 0xFF*/
    uint8_t group; /* 只有当 ptp = 0 且 dst = 0xFE 时，填组内组号, 其余时候填0*/
} dc_model_param_t;


typedef struct can_task_pool
{
    pq_t *queue;
    pthread_t *tid;//工作线程池
    uint32_t thread_count; //工作线程数
    uint64_t fr_interval_time;
} can_task_pool_t;

int32_t taskpool_init(can_task_pool_t *pool, void *(task_func)(void *), int32_t queue_size, int32_t thread_num, uint64_t fr_interval_time, const char *pth_name);
void can_test(void);

/**
 * @brief 设置模块输出电流 建议组内广播
 * @param can_id_param can帧id需指定的信息, dst ptp group
 * @param out_cur 输出电流
 * @return bool
 */
bool set_mode_out_cur(dc_model_param_t can_id_param, float out_cur);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
