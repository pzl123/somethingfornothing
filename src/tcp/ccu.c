#include "ccu.h"
#include "utils/utils.h"

#include "hv/hloop.h"
#include "hv/hsocket.h"
#include "hv/hsocket.h"
#include "hv/htime.h"

static void on_close(hio_t* io)
{
    d_log("on_close fd: %d error: %d", hio_fd(io), hio_error(io));
    d_log("ccu id: %u, ccu client disconnected.", hio_id(io));
}

static void on_recv(hio_t *io, void *buf, int32_t readbytes)
{
    (void)io;
    d_log("recv %d bytes: %.*s", readbytes, readbytes, (char *)buf);
}

/* static void on_timer(htimer_t *timer)
{
    char str[DATETIME_FMT_BUFLEN] = {0};
    datetime_t dt = datetime_now();
    datetime_fmt(&dt, str);

    printf("> ccu_server %s\n", str);
    hio_t *io = (hio_t *)hevent_userdata(timer);
    hio_write(io, str, strlen(str));
} */

static void on_accept(hio_t *io)
{
    char local_addr_str[SOCKADDR_STRLEN] = {0};
    char peer_addr_str[SOCKADDR_STRLEN] = {0};
    d_log("accept connect fd: %d [%s] <= [%s]",
          hio_fd(io),
          SOCKADDR_STR(hio_localaddr(io), local_addr_str),
          SOCKADDR_STR(hio_peeraddr(io), peer_addr_str));

    // static unpack_setting_t s_unpack_setting; /* 粘包拆包处理 */
    // s_unpack_setting.mode = UNPACK_BY_LENGTH_FIELD; /* Suitable for binary protocol 按长度字段拆包 */
    // s_unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    // s_unpack_setting.body_offset = CCU_MSG_HEAD_LEN;
    // s_unpack_setting.length_field_offset = CCU_MSG_HEAD_LEN - sizeof(uint16_t);
    // s_unpack_setting.length_field_bytes = sizeof(uint16_t);
    // s_unpack_setting.length_field_coding = ENCODE_BY_BIG_ENDIAN;
    // hio_set_unpack(io, &s_unpack_setting);

    hio_setcb_close(io, &on_close);
    hio_setcb_read(io, &on_recv);
/*     htimer_t *timer = htimer_add(hevent_loop(io), on_timer, 1000, INFINITE);
    hevent_set_userdata(timer, io); */
    hio_read(io);
}

void *ccu_server_start_internal(void *arg)
{
    (void)arg;
    hloop_t *loop = hloop_new(HLOOP_FLAG_AUTO_FREE);
    while (true)
    {
        hio_t *listen_io = hloop_create_tcp_server(loop, ETH_IP, CCU_TCP_SERVER_PORT, &on_accept);
        if(NULL == listen_io)
        {
            d_log("create listen io failed");
        }
        else
        {
            break;
        }
    }

    (void)hloop_run(loop);
    return NULL;
}

