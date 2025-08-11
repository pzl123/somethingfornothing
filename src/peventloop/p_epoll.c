#include "peventloop.h"
#include "anet.h"

#include <sys/epoll.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct pApiState
{
    int epfd;
    struct epoll_event* events;
} pApiState_t;


static int pApiCreate(pEventLoop_t *eventloop)
{
    pApiState_t *state =(pApiState_t *)malloc(sizeof(pApiState_t));
    if (NULL == state)
    {
        return -1;
    }
    state->events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * eventloop->setsize);
    if (NULL == state->events)
    {
        free(state);
        return -1;
    }
    state->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
    if (-1 == state->epfd)
    {
        free(state->events);
        free(state);
        return -1;
    }

    anetCloexec(state->epfd);
    eventloop->apidata = state;
    return 0;
}

static int pApiResize(pEventLoop_t *eventLoop, int setsize)
{
    pApiState_t *state = eventLoop->apidata;
    state->events = (struct epoll_event*)realloc(eventLoop->events, sizeof(struct epoll_event) * setsize);
    return 0;
}