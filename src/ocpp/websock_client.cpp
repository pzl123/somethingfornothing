#include "websock_client.h"
#include "utils/utils.h"

// 假设您有一个会话数据结构
struct ocpp_session
{
    int connected;
    char charge_box_id[64];
};

// 回调函数
int ocpp_1_6_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        d_log("WebSocket connected to Central System");
        break;

    case LWS_CALLBACK_RECEIVE:
        d_log("Received from steVe: %.*s", (int)len, (char *)in);
        // TODO: 解析 OCPP 消息
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        d_log("Connection error: %s", in ? (char *)in : "unknown reason");
        break;

    case LWS_CALLBACK_CLOSED:
        d_log("WebSocket closed");
        break;

    default:
        break;
    }
    return 0;
}

// 协议定义
static struct lws_protocols protocols[] =
    {
        {
            .name = "ocpp1.6",
            .callback = ocpp_1_6_cb,
            .per_session_data_size = sizeof(struct ocpp_session),
        },
        {
            .name = NULL,
            .callback = NULL,
            .per_session_data_size = 0,
        }};

// 全局变量（实际项目中应封装）
static struct lws_context *context = NULL;
static struct lws *wsi = NULL;

int32_t web_socket_client_test(void)
{
    // 1. 初始化上下文配置
    struct lws_context_creation_info ctx_info = {0};
    ctx_info.port = CONTEXT_PORT_NO_LISTEN; // 客户端
    ctx_info.iface = NULL;
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;
    ctx_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8; // 可选：校验 UTF-8

    // 2. 创建上下文
    context = lws_create_context(&ctx_info);
    if (!context)
    {
        d_log("Failed to create libwebsockets context");
        return -1;
    }

    // 3. 准备连接信息
    struct lws_client_connect_info conn_info = {0};
    conn_info.context = context;
    conn_info.address = "192.168.18.128";
    conn_info.port = 8180;
    conn_info.path = "/steve/websocket/CentralSystemService/ChargeBox1";
    conn_info.host = conn_info.address;
    conn_info.origin = conn_info.address;
    conn_info.protocol = "ocpp1.6";
    conn_info.ssl_connection = 0; // 如果 STEVe 启用了 HTTPS/WSS，改为 LCCSCF_USE_SSL

    // 4. 发起连接
    wsi = lws_client_connect_via_info(&conn_info);
    if (!wsi)
    {
        d_log("Failed to connect to server");
        lws_context_destroy(context);
        return -1;
    }

    d_log("Connecting to %s:%d%s...", conn_info.address, conn_info.port, conn_info.path);

    // 5. 事件循环（通常放在独立线程或主循环中）
    while (1)
    {
        lws_service(context, 100); // 100ms 超时
        // 可在此处插入其他任务
        usleep(10000); // 小延时避免 CPU 占满
    }

    // 清理（不会执行到这里）
    lws_context_destroy(context);
    return 0;
}