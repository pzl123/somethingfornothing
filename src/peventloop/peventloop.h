#ifndef __PEVENTLOOP_H__
#define __PEVENTLOOP_H__

#include "monotonic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define P_OK 0
#define P_ERR -1

#define P_NONE 0     /* 无事件注册 */
#define P_READABLE 1 /* 当 descriptor 可读时触发 */
#define P_WRITABLE 2 /* 当 descriptor 可写时触发 */
#define P_BARRIER 4  /* 在使用 WRITABLE（可写事件）标志时, \
                      * 如果在当前事件循环的一次迭代中已经触发了READABLE, \
                      * 就不在触发 WRITABLE 事件  \
                      * 这在你想在发送响应前先把数据持久化到磁盘、并且希望批量处理这类操作时非常有用 */

#define P_FILE_EVENTS (1 << 0)
#define P_TIME_EVENTS (1 << 1)
#define P_ALL_EVENTS (P_FILE_EVENTS | P_TIME_EVENTS)
#define P_DONT_WAIT (1 << 2)
#define P_CALL_BEFORE_SLEEP (1 << 3)
#define P_CALL_AFTER_SLEEP (1 << 4)

#define P_NOMERE -1
#define P_DELETED_EVENT_ID -1

#define P_NOTUSED(V) ((void)V)

struct pEventLoop;

/* 类型和数据结构 */
typedef void pFileProc(struct pEventLoop *eventloop, int fd, void *clientData, int mask);
typedef int pTimeProc(struct pEventLoop *eventloop, long long id, void *clientData);
typedef void pEventFinalizerProc(struct pEventLoop *eventloop, void *clientData);
typedef void pBeforeSleepProc(struct pEventLoop *eventloop);

/* 文件 事件结构 */
typedef struct pFileEvent
{
    int mask; /* one of P_(READABLE | WRITEABLE | BARRIER) */
    pFileProc *rfileProc;
    pFileProc *wfileProc;
    void *clientData;
} pFileEvent_t;


/* 时间 事件结构 */
typedef struct pTimeEvent
{
    long long id; /* 时间事件 identifier */
    monotime when;
    pTimeProc *timeProc;
    pEventFinalizerProc *finalizerProc;
    void *clientData;
    struct pTimeEvent *prev;
    struct pTimeEvent *next;
    int refcount; /* 使用引用计数来防止定时器事件在递归调用定时器事件的过程中被释放（free） */
}pTimeEvent_t;


/* 被触发的事件 */
typedef struct pFiredEvent
{
    int fd;
    int mask;
} pFiredEvent_t;

/* 基于事件的程序的状态 */
typedef struct pEventLoop
{
    int maxfd; /* 当前注册的最高 file descriptor */
    int setsize; /* 跟踪的最大文件描述符数 */
    long long timeEventNextId;
    int nevents; /* 已注册事件的大小 */
    pFileEvent_t *events; /* 已注册的事件 */
    pFileEvent_t *fired; /* 已触发的事件 */
    pTimeEvent_t *timeEventHead;
    int stop;
    void *apidata; /* 这用于polling API 特定的数据 */
    pBeforeSleepProc *beforesleep;
    pBeforeSleepProc *aftersleep;
    int flags;
    void *privdata[2];
} pEventLoop_t;

/* Prototypes 原型 */
pEventLoop_t *pCreateEventLoop(int setsize);

int pGetSetSize(const pEventLoop_t *eventLoop);

#ifdef __cplusplus
}
#endif

#endif /* __PEVENTLOOP_H__ */
