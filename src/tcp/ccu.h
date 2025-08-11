#ifndef CCU_H
#define CCU_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define ETH_IP "0.0.0.0"         /* eth0 ipv4 地址固定 */
#define CCU_TCP_SERVER_PORT 8000 /* ccu tcp 服务本地绑定端口 */
/* ccu 消息头 */
#pragma pack(push, 1)
typedef struct
{
    uint8_t head_prefix[2]; /* 帧头前缀 */
    uint8_t ccu_id;         /* 桩编号 */
    uint16_t cmd;           /* 命令 */
    uint16_t msg_id;        /* 消息 id */
    uint16_t data_len;      /* 荷载数据长度 */
} ccu_msg_head_t;
#pragma pack(pop)

#define CCU_MSG_HEAD_LEN sizeof(ccu_msg_head_t)

extern void *ccu_server_start_internal(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* CCU_H */
