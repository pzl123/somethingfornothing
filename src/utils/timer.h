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


int32_t p_timer_init(void);
Timer_handle_t p_timer_create(uint64_t timeout_ms, int32_t repeat, timeout_cb cb, void *userdata);
int32_t p_timerSetParam(Timer_handle_t hdl, void *userdata);
int32_t p_timer_start(Timer_handle_t hdl);
int32_t p_timer_stop(Timer_handle_t hdl);
int32_t p_timer_del(Timer_handle_t *p_hdl);

int32_t p_timer_start_loop_pthread(void *quit_ctl);
void *p_timer_mian_loop(void *quit_ctl);
void p_timer_loop_get_next_min_timeout(struct timeval *tv);
void p_timer_loop_once(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__ */
