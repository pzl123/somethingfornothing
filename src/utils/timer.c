#include "timer.h"
#include "utils/utils.h"

#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <stdio.h>




typedef enum
{
    TIMER_IDLE = 0,
    TIMER_RUNNING,
    TIMER_STOP
} timer_status_e;

struct timer_handle_struct
{
    struct timer_handle_struct *prev;
    struct timer_handle_struct *next;
    struct timespec abs_timeout;
    timeout_cb cb;
    uint64_t timeout_ms;
    void *userdata;
    int32_t repeat;
    int32_t timeout_count;
    timer_status_e status;
};


static int32_t g_ctrl_fd = -1;
static Timer_handle_t g_timer_list_head;
static pthread_mutex_t g_timer_list_mutex;
static pthread_t g_main_loop_thread_fd;

static inline void _timer_add_abstimeout(struct timespec *abstimeout, uint64_t timeout_ms)
{
    if (NULL == abstimeout)
    {
        d_log("add abstime error");
    }
    abstimeout->tv_sec += (timeout_ms/1000);
    abstimeout->tv_nsec += ((timeout_ms % 1000) * 1000000);
    if (abstimeout->tv_nsec > 1000000000)
    {
        abstimeout->tv_sec++;
        abstimeout->tv_nsec -= 1000000000;
    }
}

static inline void _timer_set_abstimeout(struct timespec *abstimeout, uint64_t timeout_ms)
{
    clock_gettime(CLOCK_MONOTONIC, abstimeout);
    _timer_add_abstimeout(abstimeout, timeout_ms);
}

static inline void _timer_post_timeout_signal(void)
{
    if (g_ctrl_fd > 0)
    {
        uint64_t val = 1;
        (void)write(g_ctrl_fd, &val, sizeof(val));
    }
}

/*
* first > second -> 1
* first < second -> -1
* first == second -> 0
*/
static inline int32_t _timespec_compare(struct timespec *first, struct timespec *second)
{
    if (first->tv_sec > second->tv_sec)
    {
        return 1;
    }

    if (first->tv_sec < second->tv_sec)
    {
        return -1;
    }

    if (first->tv_nsec == second->tv_nsec)
    {
        return 0;
    }

    return (first->tv_nsec > second->tv_nsec) ? 1 : -1;
}

static inline bool _timer_is_inlist(Timer_handle_t hdl)
{
    for (Timer_handle_t cur = g_timer_list_head; NULL != cur; cur = cur->next)
    {
        if (cur == hdl)
        {
            return true;
        }
    }

    return false;
}

static inline void _timer_list_remove(Timer_handle_t hdl)
{
    if ((NULL == hdl->prev) && (NULL == hdl->next)) /* only one */
    {
        g_timer_list_head = NULL;
    }
    else if (NULL == hdl->prev)  /* first node */
    {
        hdl->next->prev = NULL;
        g_timer_list_head = hdl->next;
    }
    else if (NULL == hdl->next) /* last one */
    {
        hdl->prev->next = NULL;
    }
    else
    {
        hdl->next->prev = NULL;
        hdl->prev->next = NULL;
    }
}

static inline void _timer_list_insert_after(Timer_handle_t cur, Timer_handle_t hdl)
{
    Timer_handle_t next = cur->next;
    hdl->prev = cur;
    hdl->next = next;
    cur->next = hdl;
    if (NULL != next)
    {
        next->prev = hdl;
    }
    return;
}

static inline void _timer_list_insert_before(Timer_handle_t cur, Timer_handle_t hdl)
{
    Timer_handle_t prev = cur->prev;
    if (NULL == prev)
    {
        g_timer_list_head = hdl;
        hdl->prev = NULL;
        hdl->next = cur;

        cur->prev = hdl;
    }
    else
    {
        hdl->prev = prev;
        hdl->next = cur;

        prev->next = hdl;
        cur->prev = hdl;
    }

    return;
}

static inline int32_t _timer_list_add(Timer_handle_t hdl)
{
    if (NULL == g_timer_list_head)
    {
        g_timer_list_head = hdl;
        hdl->prev = NULL;
        hdl->next = NULL;
        return 0;
    }

    int32_t index = 0;
    for (Timer_handle_t cur = g_timer_list_head; cur != NULL; cur = cur->next, index++)
    {
        if (_timespec_compare(&cur->abs_timeout, &hdl->abs_timeout) > 0)
        {
            /* should insert before cur node */
            _timer_list_insert_before(cur, hdl);
            return index;
        }

        if (cur->next == NULL)
        {
            _timer_list_insert_after(cur, hdl);
            break;
        }
    }
	return index;

}

static void p_timer_loop_once(void)
{
    if (NULL == g_timer_list_head)
    {
        return;
    }
    struct timespec curtime;
    clock_gettime(CLOCK_MONOTONIC, &curtime);

    // check for timer timeout
    pthread_mutex_lock(&g_timer_list_mutex);
    Timer_handle_t cur = g_timer_list_head;
    while (NULL != cur)
    {
        Timer_handle_t next = cur->next;
        if (_timespec_compare(&curtime, &cur->abs_timeout) < 0)
        {
            break;
        }
        //timeout
        if (cur->status != TIMER_RUNNING)
        {
            _timer_list_remove(cur);
            cur = next;
            continue;
        }
        // d_log("TIMER TIMEOUT %p timeout_ms:%d @ sec:%ld, nsec:%ld, repeat:%d/%d", cur, cur->timeout_ms, curtime.tv_sec, curtime.tv_nsec, cur->timeout_count + 1, cur->repeat);
        if (cur->cb)
        {
            cur->cb(cur->userdata);
        }
        cur->timeout_count++;
        _timer_list_remove(cur);
        if ((cur->timeout_count < cur->repeat) || (cur->repeat < 0))
        {
            // need restart the loop timer
#if ABSOLUTE_STRICT_INTERVAL_TIMEOUT == 2 // most strict
            _timer_add_abstimeout(&cur->abstimeout, cur->timeout_ms);
#elif ABSOLUTE_STRICT_INTERVAL_TIMEOUT == 1 // strict
            cur->abs_timeout = curtime;
            _timer_add_abstimeout(&cur->abs_timeout, cur->timeout_ms);
#else
            _timer_set_abstimeout(&cur->abs_timeout, cur->timeout_ms);
#endif
            _timer_list_add(cur);
        }
        else
        {
            cur->status = TIMER_STOP;
        }
        cur = next;
    }
    pthread_mutex_unlock(&g_timer_list_mutex);

}

static void p_timer_loop_get_next_min_timeout(struct timeval *tv)
{
    Timer_handle_t cur = g_timer_list_head;
    if (NULL != cur)
    {
        struct timespec time_now;
        clock_gettime(CLOCK_MONOTONIC, &time_now);
        tv->tv_sec = cur->abs_timeout.tv_sec - time_now.tv_sec;
        tv->tv_usec = (cur->abs_timeout.tv_nsec - time_now.tv_nsec)/1000;
        if (tv->tv_usec < 0)
        {
            tv->tv_usec += 1000000;
            tv->tv_sec--;
        }
        if ((tv->tv_sec < 0) || ((tv->tv_sec == 0) && (tv->tv_usec < 1000)))
        {
            tv->tv_sec = 0;
            tv->tv_usec = 1000;
        }
    }
    else
    {
        tv->tv_sec = 10;
        tv->tv_usec = 0;
    }
}

static void *p_timer_mian_loop(void *quit_ctl)
{
    struct timeval tv;
    fd_set rfds;
    do
    {
        FD_ZERO(&rfds);
        FD_SET(g_ctrl_fd, &rfds);
        p_timer_loop_get_next_min_timeout(&tv);
        // d_log("TIMER in sec:%ld, usec:%ld", tv.tv_sec, tv.tv_usec);
        int ret = select(g_ctrl_fd + 1, &rfds, NULL, NULL, &tv);
        if ((ret > 0) && (FD_ISSET(g_ctrl_fd, &rfds)))
        {
            uint64_t val;
            (void)read(g_ctrl_fd, &val, sizeof(val));
        }
        p_timer_loop_once();
    } while (0 == *(int32_t *)quit_ctl);
    d_log("TIMER MAIN LOOP EXIT");
    return quit_ctl;
}

static int32_t p_timer_start_loop_pthread(void *quit_ctl)
{
    int32_t ret = pthread_create(&g_main_loop_thread_fd, NULL, &p_timer_mian_loop, quit_ctl);
    if (0 == ret)
    {
        pthread_detach(g_main_loop_thread_fd);
    }
    return ret;
}

int32_t p_timer_init(void *quit_ctl)
{
    pthread_mutex_init(&g_timer_list_mutex, NULL);
    g_timer_list_head = NULL;
    g_ctrl_fd = eventfd(0,0);
    p_timer_start_loop_pthread(quit_ctl);
    return g_ctrl_fd;
}

Timer_handle_t p_timer_add(uint64_t timeout_ms, int32_t repeat, timeout_cb cb, void *userdata)
{
    if ((timeout_ms <= 0) || (NULL == cb) || (NULL == userdata))
    {
        d_log("create timer paramater error");
        return NULL;
    }

    Timer_handle_t hdl = (Timer_handle_t)malloc(sizeof(struct timer_handle_struct));
    if (NULL == hdl)
    {
        d_log("malloc hdl error");
        return NULL;
    }

    hdl->timeout_ms = timeout_ms;
    hdl->cb = cb;
    hdl->userdata = userdata;
    hdl->repeat = repeat;
    hdl->timeout_count = 0;
    hdl->status = TIMER_IDLE;
    hdl->prev = NULL;
    hdl->next = NULL;

    p_timer_start(hdl);
    return hdl;
}

int32_t p_timerSetParam(Timer_handle_t hdl, void *userdata)
{
    if (NULL == hdl)
    {
        d_log("set paramater error");
        return -1;
    }
    hdl->userdata = userdata;
    return 0;
}

int32_t p_timer_start(Timer_handle_t hdl)
{
    if (NULL == hdl)
    {
        d_log("%s paramater error", __func__);
        return -1;
    }

    _timer_set_abstimeout(&hdl->abs_timeout, hdl->timeout_ms);
    hdl->timeout_count = 0;
    hdl->status = TIMER_RUNNING;

    pthread_mutex_lock(&g_timer_list_mutex);
    /* remove it first if it is already in the list, so we can start(restart) the timer again
       如果它已经在列表中，请先将其删除，这样我们就可以再次启动（重新启动）计时器 */
    if (_timer_is_inlist(hdl))
    {
        _timer_list_remove(hdl);
    }

    /* add to the list stored */
    if (0 == _timer_list_add(hdl))
    {
        /* if added to the first of the list, then the timeout need refresh, timeout now */
        _timer_post_timeout_signal();
    }
    pthread_mutex_unlock(&g_timer_list_mutex);
    return 0;
}

int32_t p_timer_stop(Timer_handle_t hdl)
{
    if (NULL == hdl)
    {
        return -1;
    }

    hdl->status = TIMER_STOP;

    /* remove from the list */
    pthread_mutex_lock(&g_timer_list_mutex);
    if(_timer_is_inlist(hdl))
    {
        _timer_list_remove(hdl);
    }
    pthread_mutex_unlock(&g_timer_list_mutex);
    return 0;
}

static void free_hdl(Timer_handle_t hdl)
{
    if (NULL != hdl->userdata)
    {
        free(hdl->userdata);
    }
}

int32_t p_timer_del(Timer_handle_t *p_hdl)
{
    if ((NULL == p_hdl) || (NULL == *p_hdl))
    {
        return -1;
    }
    Timer_handle_t hdl = *p_hdl;
    hdl->status = TIMER_STOP;

    /* remove from the list */
    pthread_mutex_lock(&g_timer_list_mutex);
    if (_timer_is_inlist(hdl))
    {
        _timer_list_remove(hdl);
    }
    pthread_mutex_unlock(&g_timer_list_mutex);
    free_hdl(hdl);
    *p_hdl = NULL;
    return 0;
}

