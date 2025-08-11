#include "/home/zlgmcu/project/learnC++/include/hv/hloop.h"
#include "/home/zlgmcu/project/learnC++/include/hv/hsocket.h"
#include "/home/zlgmcu/project/learnC++/include/hv/htime.h"



#include <stdio.h>

#define ETH_IP "172.30.1.55"      /* eth0 ipv4 地址固定 */
#define CCU_TCP_CLIENT_PORT 8000 /* ccu tcp client本地绑定端口 */

void on_timer(htimer_t *timer)
{
    char str[DATETIME_FMT_BUFLEN] = {0};
    datetime_t dt = datetime_now();
    datetime_fmt(&dt, str);

    printf("> ccu_client %s\n", str);
    hio_t *io = (hio_t *)hevent_userdata(timer);
    hio_write(io, str, strlen(str));
}

void on_close(hio_t *io)
{
    hio_close(io);
}

void on_recv(hio_t *io, void *buf, int readbytes)
{
    printf("< %.*s\n", readbytes, (char *)buf);
}

void on_connect(hio_t *io)
{
    hio_setcb_close(io, on_close);
    hio_setcb_read(io, on_recv);
    hio_read(io);

    htimer_t *timer = htimer_add(hevent_loop(io), on_timer, 1000, INFINITE);
    hevent_set_userdata(timer, io);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    hloop_t *loop = hloop_new(0);
    hio_t *listenio = hloop_create_tcp_client(loop, ETH_IP, CCU_TCP_CLIENT_PORT, on_connect, NULL);
    if (listenio == NULL)
    {
        return -20;
    }
    hloop_run(loop);
    hloop_free(&loop);
    return 0;
}

/* gcc ccu_client.c -o ccu_client -I../../include -L../../lib -lhv_static -lpthread */