#include "utils/utils.h"
#include "hv/hloop.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <net/if.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/prctl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#define POST_CHARGE_LOG_DURATION (10 * 1000) // 停止充电后继续记录日志的时间（秒）

#define MAX_LOG_MSG_LEN (77)
#define INIT_LOG_QUEUE_SIZE (20)
static bool g_was_charging = false;
static uint64_t g_stop_charging_time = 0;

typedef struct
{
    int32_t sock_fd;
    bool log_flag;
} need_t;

FILE *g_fd = NULL;

typedef struct
{
    char log_msg_queue[2][INIT_LOG_QUEUE_SIZE][MAX_LOG_MSG_LEN];
    int32_t cur_buf_index;
    int32_t write_index;
    int32_t cnt[2];
    char *log_path;
    char *log_name;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} log_queue_t;
static log_queue_t g_log_queue;

static void get_current_time_str(char *buffer, uint64_t buf_size)
{
    struct timeval tv;
    (void)gettimeofday(&tv, NULL);

    struct tm *tm_info = localtime(&tv.tv_sec);
    (void)strftime(buffer, buf_size, "%Y-%m-%d %H:%M:%S", tm_info);
    uint64_t len = strlen(buffer);
    (void)snprintf(buffer + len, buf_size - len, ".%06ld", tv.tv_usec);
}

static int32_t read_can_msg_log(int32_t sock_fd, char *str, int32_t len)
{
    char log_msg[60];
    (void)memset(log_msg, 0, sizeof(log_msg));

    struct can_frame frame;
    (void)memset((void *)&frame, 0, sizeof(struct can_frame));

    int32_t ret = read(sock_fd, &frame, sizeof(struct can_frame));
    if (ret != sizeof(struct can_frame))
    {
        return -1;
    }
    (void)snprintf(log_msg, sizeof(log_msg), "mcan1  %08x  [8]  %02x %02x %02x %02x %02x %02x %02x %02x", frame.can_id,
                frame.data[0], frame.data[1], frame.data[2], frame.data[3],
                frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
    char time_str[32];
    get_current_time_str(time_str, sizeof(time_str));

    (void)snprintf(str, len, "(%s)  %s\n", time_str, log_msg);
    return 0;
}


static int32_t init_can_log_queue(log_queue_t *q, const char *log_path, const char *log_name)
{
    (void)memset(q, 0, sizeof(log_queue_t));
    q->log_path = log_path;
    q->log_name = log_name;
    (void)pthread_mutex_init(&q->mutex, NULL);
    (void)pthread_cond_init(&q->cond, NULL);
    return 0;
}

static FILE *open_file(const char* log_path, const char *log_name)
{
    if ((NULL == log_path) || (NULL == log_name))
    {
        e_log("Invalid path or name");
        return NULL;
    }


    struct stat st;
    if (0 != stat(log_path, &st) || !S_ISDIR(st.st_mode))
    {
        mode_t mode = 0755;
        if (0 != mkdir(log_path, mode))
        {
            e_log("failed to create directory:%s", log_path);
            return NULL;
        }
    }
    char target_path[256] = {'\0'};
    (void)snprintf(target_path, sizeof(target_path), "%s/%s", log_path, log_name);
    FILE *fp = fopen(target_path, "a+");
    if (NULL == fp)
    {
        return NULL;
    }
    return fp;
}

static void *log_write_pthread(void *arg)
{
    (void)prctl(PR_SET_NAME, __FUNCTION__);
    log_queue_t* q = (log_queue_t*)arg;
    if ((NULL == q))
    {
        return NULL;
    }

    char target_file[256] = {'\0'};
    snprintf(target_file, sizeof(target_file), "%s/%s", q->log_path, q->log_name);

    while (1)
    {
        struct stat st = {0};
        if (-1 == stat(target_file, &st))
        {
            g_fd = open_file(q->log_path, q->log_name);
        }

        pthread_mutex_lock(&q->mutex);
        while (q->cnt[1 - q->cur_buf_index] == 0)
        {
            pthread_cond_wait(&q->cond, &q->mutex);
        }

        // uint64_t time_aaa = gettime_msec();
        for (int32_t i = 0; i < q->cnt[1 - q->cur_buf_index]; i++)
        {
            fwrite(q->log_msg_queue[1 - q->cur_buf_index][i], strlen(q->log_msg_queue[1 - q->cur_buf_index][i]), 1, g_fd);
            // d_log("%d %s", i, q->log_msg_queue[1 - q->cur_buf_index][i]);
        }
        (void)fflush(g_fd);
        // d_log("consumer once buffer[%d] use time %ld", (1 - q->cur_buf_index), gettime_msec() - time_aaa);
        q->cnt[1 - q->cur_buf_index] = 0;
        pthread_mutex_unlock(&q->mutex);
    }

    return NULL;
}

static int32_t double_buffer_push(log_queue_t *q, const char *msg)
{
    if ((NULL == q) || (NULL == msg))
    {
        return -1;
    }
    pthread_mutex_lock(&q->mutex);
    if (q->cnt[q->cur_buf_index] >= INIT_LOG_QUEUE_SIZE)
    {
        // d_log("productor once buffer[%d] full, cnt is %d, change", q->cur_buf_index, q->cnt[q->cur_buf_index]);
        q->cur_buf_index = 1 - q->cur_buf_index;
        q->cnt[q->cur_buf_index] = 0;
        q->write_index = 0;
        pthread_cond_signal(&q->cond);
    }

    (void)strncpy(q->log_msg_queue[q->cur_buf_index][q->write_index], msg, MAX_LOG_MSG_LEN);
    q->cnt[q->cur_buf_index] += 1;
    q->write_index = (q->write_index + 1) % INIT_LOG_QUEUE_SIZE;
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

static void *dy_candump_pthread(void *arg)
{
    (void)prctl(PR_SET_NAME, __FUNCTION__);
    need_t *msg = (need_t *)arg;
    int32_t sock_fd = msg->sock_fd;
    bool *log_flag = &msg->log_flag;
    uint64_t current_time = 0;

    while (1)
    {
        current_time = gettime_msec();
        if ((true == *log_flag)&& (false == g_was_charging))
        {
            g_was_charging = true;
        }
        else if ((false == *log_flag) && (true == g_was_charging))
        {
            g_was_charging = false;
            g_stop_charging_time = current_time;
        }

        if (true == *log_flag)
        {
            char msg[MAX_LOG_MSG_LEN] = {'\0'};
            read_can_msg_log(sock_fd, msg, MAX_LOG_MSG_LEN);
            double_buffer_push(&g_log_queue, msg);
        }
        else
        {
            usleep(100 * 1000);
        }
    }
    return NULL;
}


void dy_candump(const char* can_name, const char *log_path, const char *log_name, bool log_flag)
{
    if (-1 == init_can_log_queue(&g_log_queue, log_path, log_name))
    {
        return;
    }

    g_fd = open_file(g_log_queue.log_path, g_log_queue.log_name);
    if (NULL == g_fd)
    {
        e_log("NULL == g_fd");
    }

    static __thread int32_t sock_fd;
    sock_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock_fd < 0)
    {
        e_log("socket error");
        return;
    }
    int32_t flags = fcntl(sock_fd, F_GETFL, 0);
    if (fcntl(sock_fd, F_SETFL, (uint32_t)flags & ~(uint32_t)O_NONBLOCK) < 0)
    {
        e_log("FCNTL set non blocking error");
        (void)close(sock_fd);
        return;
    }

    struct ifreq ifr;
    (void)strcpy(ifr.ifr_name, can_name);
    int32_t ret = ioctl(sock_fd, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        e_log("%s ioctl error:%s", ifr.ifr_name, strerror(errno));
        (void)close(sock_fd);
        return;
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        e_log("bind error");
        return;
    }

    int32_t loopback = 1;
    (void)setsockopt(sock_fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

    pthread_t tid1;
    (void)pthread_create(&tid1, NULL, &log_write_pthread, (void *)&g_log_queue);
    (void)pthread_detach(tid1);

    need_t *arg = (need_t *)malloc(sizeof(need_t));
    if (NULL == arg)
    {
        e_log("malloc failed");
        return;
    }
    arg->sock_fd = sock_fd;
    arg->log_flag = log_flag;

    pthread_t tid;
    (void)pthread_create(&tid, NULL, &dy_candump_pthread, (void *)arg);
    (void)pthread_detach(tid);
}

