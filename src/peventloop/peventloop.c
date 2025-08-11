#include "peventloop.h"

#include <stdlib.h>
#include <string.h>

#include "p_epoll.c"

#define INITIAL_EVENT 1024
static void free_eventloop(pEventLoop_t *eventloop)
{
    if (eventloop)
    {
        if (eventloop->events)
        {
            free(eventloop->events);
        }
        if (eventloop->fired)
        {
            free(eventloop->fired);
        }
        free(eventloop);
    }
}

pEventLoop_t *pCreateEventLoop(int setsize)
{
    pEventLoop_t *eventloop;
    int i;

    monotonicInit();
    if ((eventloop = (pEventLoop_t *)malloc(sizeof(*eventloop))) == NULL)
    {
        free_eventloop(eventloop);
        return NULL;
    }

    eventloop->nevents = (setsize < INITIAL_EVENT) ? setsize : INITIAL_EVENT;
    eventloop->events = (pFileEvent_t *)malloc(sizeof(pFileEvent_t) * eventloop->nevents);
    eventloop->fired = (pFileEvent_t *)malloc(sizeof(pFileEvent_t) * eventloop->nevents);
    if ((eventloop->events == NULL) || (eventloop->fired == NULL))
    {
        free_eventloop(eventloop);
        return NULL;
    }

    eventloop->setsize = setsize;
    eventloop->timeEventHead = NULL;
    eventloop->timeEventNextId = 0;
    eventloop->stop = 0;
    eventloop->maxfd = 0;
    eventloop->beforesleep = NULL;
    eventloop->aftersleep = NULL;
    eventloop->flags = 0;
    (void)memset(eventloop->privdata, 0, sizeof(eventloop->privdata));
    if (-1 == pApiCreate(eventloop))
    {
        free_eventloop(eventloop);
        return NULL;
    }
    return eventloop;
}


int pGetSetSize(const pEventLoop_t *eventLoop)
{
    return eventLoop->setsize;
}

/*
 * 提示事件处理尽快更改等待超时时间。
 * 注意：这仅仅意味着你开启/关闭了全局的 AE_DONT_WAIT 标志。
 */

void pSetDontWait(pEventLoop_t *eventLoop, int noWait)
{
    if (noWait)
    {
        eventLoop->flags |= P_DONT_WAIT;
    }
    else
    {
        eventLoop->flags &= ~P_DONT_WAIT;
    }
}

/*
 * 调整事件循环event loop 的最大文件描述符集合大小（最大支持的文件描述符数量）。
 * 如果请求的新大小小于当前的大小，但此时已经有某个文件描述符被使用，
 * 且该描述符大于等于新大小减一（>= requested_set_size - 1），
 * 则返回 AE_ERR，且完全不执行此次操作。
 *
 * 否则返回 AE_OK，表示操作成功。
 */
int pResizeSetSize(pEventLoop_t *eventLoop, int setsize)
{
    if (setsize == eventLoop->setsize) return P_OK;
    if (eventLoop->maxfd >= setsize) return P_ERR;
/*     if () */
}
