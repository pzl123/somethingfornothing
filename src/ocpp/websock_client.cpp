// websock_client.cpp
#include "websock_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* ---------------- 全局变量 ---------------- */
static struct lws_context *client_context = NULL;
static struct lws_context *server_context = NULL;

static volatile int server_running = 1;
static volatile int client_running = 1;

static struct lws *client_wsi = NULL; // 保存客户端连接句柄

/* ---------------- WebSocket 协议定义 ---------------- */
static int
callback_test(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
    {
        printf("[wsi=%p] 连接已建立\n", (void *)wsi);
        break;
    }

    case LWS_CALLBACK_RECEIVE:
    {
        printf("[wsi=%p] 收到消息: %.*s\n", (void *)wsi, (int)len, (char *)in);
        // 回显消息
        unsigned char buf[LWS_PRE + len];
        memcpy(&buf[LWS_PRE], in, len);
        lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
        break;
    }

    // ✅ 新增：处理 HTTP 请求，允许升级为 WebSocket
    case LWS_CALLBACK_HTTP:
    {
        char path[128] = {0};
        int n = lws_hdr_copy(wsi, path, sizeof(path), WSI_TOKEN_HEAD_URI);
        if (n < 0) {
            // 读取 URI 失败
            lws_return_http_status(wsi, 500, NULL);
            return -1;
        }
    
        printf("服务端收到 HTTP 请求，路径: %s\n", path);
    
        // 只允许 /test 路径升级为 WebSocket
        if (strcmp(path, "/test") == 0) {
            return 0; // ✅ 允许 libwebsockets 继续处理升级
        }
    
        // 其他路径返回 404
        lws_return_http_status(wsi, 404, NULL);
        return -1;
    }

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
        printf("客户端连接失败: %s\n", in ? (char *)in : "unknown");
        client_running = 0;
        break;
    }

    case LWS_CALLBACK_CLOSED:
    case LWS_CALLBACK_CLIENT_CLOSED:
    {
        printf("[wsi=%p] 连接已关闭\n", (void *)wsi);
        if (wsi == client_wsi)
        {
            client_wsi = NULL;
            client_running = 0;
            break;
        }
    }

    default:
        break;
    }

    return 0;
}

/* WebSocket 协议映射 */
static const struct lws_protocols protocols[] = {
    {
        .name = "http", // 必须的HTTP协议，用于处理基础HTTP请求
        .callback = callback_test,
        .per_session_data_size = 0,
    },
    {
        .name = "test-protocol", // WebSocket协议
        .callback = callback_test,
        .per_session_data_size = 0,
    },
    {NULL, NULL, 0, 0} /* 结束标记 */
};

/* ---------------- 服务端线程 ---------------- */
void *server_thread(void *arg)
{
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = 8000;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_DISABLE_IPV6;

    server_context = lws_create_context(&info);
    if (!server_context)
    {
        fprintf(stderr, "❌ 服务端上下文创建失败\n");
        return NULL;
    }

    printf("✅ 服务端启动，监听 ws://localhost:8000/test\n");

    while (server_running)
    {
        lws_service_tsi(server_context, 50, 0); // 使用线程安全接口
    }

    lws_context_destroy(server_context);
    server_context = NULL;
    printf("⏹️ 服务端已停止\n");
    return NULL;
}

/* ---------------- 客户端线程 ---------------- */
void *client_thread(void *arg)
{
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo;
    memset(&info, 0, sizeof(info));
    memset(&ccinfo, 0, sizeof(ccinfo));

    info.options = LWS_SERVER_OPTION_DISABLE_IPV6;
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    client_context = lws_create_context(&info);
    if (!client_context)
    {
        fprintf(stderr, "❌ 客户端上下文创建失败\n");
        return NULL;
    }

    printf("✅ 客户端线程启动，准备连接服务端...\n");
    sleep(1); // 等待服务端启动

    ccinfo.context = client_context;
    ccinfo.address = "localhost";
    ccinfo.port = 8000;
    ccinfo.path = "/test";               // 请求路径
    ccinfo.host = "localhost";
    ccinfo.origin = "localhost";
    ccinfo.protocol = "test-protocol";   // ✅ 必须匹配 protocols[0].name
    ccinfo.ietf_version_or_minus_one = -1;

    struct lws *wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi)
    {
        fprintf(stderr, "❌ 客户端连接失败\n");
        lws_context_destroy(client_context);
        client_context = NULL;
        client_running = 0;
        return NULL;
    }

    client_wsi = wsi;
    printf("✅ 客户端已连接到服务端 [wsi=%p]\n", (void *)wsi);

    int count = 0;
    while (client_running && count < 5)
    {
        char msg[64];
        sprintf(msg, "Hello from client %d", count);
        unsigned char buf[LWS_PRE + 64];
        int n = strlen(msg);
        memcpy(&buf[LWS_PRE], msg, n);
        lws_write(wsi, &buf[LWS_PRE], n, LWS_WRITE_TEXT);
        count++;

        lws_service_tsi(client_context, 50, 0);
        sleep(1);
    }

    // 发送关闭帧并释放连接
    if (client_wsi)
    {
        lws_close_free_wsi(client_wsi, LWS_CLOSE_STATUS_NORMAL, NULL);
        client_wsi = NULL;
    }

    sleep(1); // 等待关闭完成

    lws_context_destroy(client_context);
    client_context = NULL;
    printf("⏹️ 客户端已停止\n");
    return NULL;
}

/* ---------------- 主测试函数 ---------------- */
int websocket_test(void)
{
    pthread_t tid_server, tid_client;

    // 创建服务端线程
    if (pthread_create(&tid_server, NULL, server_thread, NULL) != 0)
    {
        perror("❌ 创建服务端线程失败");
        return 1;
    }

    // 创建客户端线程
    if (pthread_create(&tid_client, NULL, client_thread, NULL) != 0)
    {
        perror("❌ 创建客户端线程失败");
        server_running = 0;
        pthread_join(tid_server, NULL);
        return 1;
    }

    // 等待客户端完成
    pthread_join(tid_client, NULL);

    // 停止服务端
    server_running = 0;
    pthread_join(tid_server, NULL);

    printf("🔚 所有线程已退出\n");
    return 0;
}