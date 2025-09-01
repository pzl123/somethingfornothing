#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include <sys/time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define INFINITE    (uint32_t)-1

#define ABSOLUTE_STRICT_INTERVAL_TIMEOUT  1

#define TIMER_INFINITE_LOOP    (-1)
#define TIMER_LOOP_ONCE        1

/*
* 将 ABSOLUTE_STRICT_INTERVAL_TIMEOUT 设置为 1。Loop 超时间隔，包括 Callback 执行时间，更严格
* 将 ABSOLUTE_STRICT_INTERVAL_TIMEOUT 设置为 0。循环超时间隔（不包括回调执行时间）
*/
typedef struct timer_handle_struct *Timer_handle_t;

typedef void (*timeout_cb)(void *userdata);

/**
 * @brief 初始化定时器,并且开启一个单独线程给定时器
 *
 * @param quit_ctl 0 定时器线程loop 非0退出
 * @return int32_t 返回一个fd
 */
int32_t p_timer_init(void *quit_ctl);

/**
 * @brief 创建一个定时任务, 默认已使用p_timer_start开始, 只创建不想使用, 创建完后使用p_timer_stop停止
 *
 * @param timeout_ms 定时时间 ms
 * @param repeat 重复次数 TIMER_INFINITE_LOOP 无限重复 or 一个大于0的正整数
 * @param cb 定时回调
 * @param userdata 回调数据
 * @return Timer_handle_t 返回一个定时器
 */
Timer_handle_t p_timer_add(uint64_t timeout_ms, int32_t repeat, timeout_cb cb, void *userdata);
int32_t p_timerSetParam(Timer_handle_t hdl, void *userdata);
int32_t p_timer_start(Timer_handle_t hdl);
int32_t p_timer_stop(Timer_handle_t hdl);
int32_t p_timer_del(Timer_handle_t *p_hdl);

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__ */
