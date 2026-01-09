#ifndef WEBSOCK_CLIENT_H
#define WEBSOCK_CLIENT_H

/*
调用 Connect()
    解析 URL → 设置 m_url, m_protocol, m_credentials
    创建 lws_context
    启动 m_thread → 运行 process()
process() 线程循环
    while (!m_processEnd) {
        lws_service(m_context, 10); // 处理事件
        // 可能检查重连、ping 等
    }

LWS 触发 eventcb
    LWS_CALLBACK_CLIENT_ESTABLISHED → m_connected = true; 通知 wsClientConnected()
    LWS_CALLBACK_RECEIVE → 拼接分片帧 → 完整后调用 wsClientDataReceived()
    LWS_CALLBACK_CLIENT_WRITEABLE → 从 m_queue 取消息发送
发送数据
    外部调用 send() → 入队 SendMsg*
    LWS 在 WRITEABLE 回调中出队并 lws_write()
断开/重连
    网络错误 → 通知 wsClientFailed()
    启动重连定时器（connectcb）
 */

#include "websocket/libwebsockets.h"
#include "credentials.h"
#include "ocpp/tool/url/Url.h"
#include "ocpp/common/Queue.h"

#include <stdint.h>
#include <string>
#include <chrono>
#include <thread>

namespace ocpp1_6
{
    namespace client
    {
        struct Credentials
        {
            std::string user;
            std::string password;

            std::string tls12_cipher_list;
            std::string tls13_cipher_list;
            bool encoded_pem_certificates = false;

            std::string server_certificate_ca;
            std::string client_certificate;
            std::string client_certificate_private_key;
            std::string client_certificate_private_key_passphrase;

            bool allow_selfsigned_certificates = false;
            bool allow_expired_certificates = false;
            bool accept_untrusted_certificates = false;
            bool skip_server_name_check = false;

            std::string server_name;
        };

        class WebSocketClient;
        struct websocketMember
        {
            struct lws_context* m_context; /* LWS 上下文（全局）管理所有连接、定时器、日志等 */
            lws_log_cx_t m_log_context;
            lws_sorted_usec_list_t m_sul; /* 用于lws_sul_schedule调度 */
            struct lws* m_wsi; /* WebSocket 实例（per-connection） 代表当前 WebSocket 连接 */
            WebSocketClient *m_self; /* 反向指针 */
        };

        class WebSocketClient
        {
        public:
            WebSocketClient();
            ~WebSocketClient();

            bool Connect(const std::string& url,
                         const std::string& protocol,
                         const Credentials& credentials,
                         std::chrono::milliseconds connect_timeout,
                         std::chrono::milliseconds retry_interval = std::chrono::milliseconds(5000),
                         std::chrono::milliseconds ping_interval = std::chrono::milliseconds(5000));

            bool disConnect(bool clearMsgFlag);
            bool isConnect();
            bool send(const void* data, uint64_t size);

            class IListener
            {
                public:
                virtual ~IListener() {}

                virtual void wsClientConnected() = 0;
                virtual void wsClientFailed() = 0;
                virtual void wsClientDisconnected() = 0;
                virtual void wsClientError() = 0;

                /* ws收到消息, 用户在此实现自己的解释逻辑 */
                virtual void wsClientDataReceived(const void *data, uint64_t size) = 0;
            };
            void registerListener(IListener *listener);

        private:
            struct SendMsg
            {
                uint64_t size;
                unsigned char *data;
                unsigned char *payload;
                SendMsg(const void *_data, uint64_t _size):size(_size)
                {
                    data = new unsigned char[LWS_PRE + _size]; // 分配总内存
                    (void)memcpy(&data[LWS_PRE], _data, _size);  // 复制用户数据
                    payload = &data[LWS_PRE]; // payload 指向用户数据区
                }
                ~SendMsg() { delete[] data; }
            };

            struct websocketMember m_websocketMember;

            IListener* m_listener;
            std::thread *m_thread;
            bool m_processEnd; /* 标志位，指示是否结束当前操作，如网络连接或线程运行 */

            uint32_t m_retry_interval_ms;
            uint16_t m_ping_interval_s;
            bool m_connection_error_notified;

            Url m_url;

            std::string m_protocol; /* web实例使用的协议 由外部调用者传入 */
            Credentials m_credentials;
            static const struct lws_protocols m_protocols[]; /* 静态成员变量 websocket协议数组 具体协议的实现方式 */
            uint16_t m_retry_count;

            bool m_connected;
            ocpp1_6::common::Queue<SendMsg*> m_queue;

            uint8_t* m_fragmented_frame;
            size_t m_fragmented_frame_size;
            size_t m_fragmented_frame_index;

            void setupSSLInfo(struct lws_context_creation_info *info);

            void process();

            void beginFragmentedFrame(size_t frame_size);
            void appendFragmentedDate(const void* data, size_t size);
            void releaseFragmentedFrame();

            static void connectcb(struct lws_sorted_usec_list* sul); /* 真正的websocket 连接函数 由lws_sul_schedule调度 */
            static int eventcb(struct lws* wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) noexcept;
        };
    } // namespace client

} // namespace ocpp1_6


int32_t web_socket_client_test(void);

#endif /* WEBSOCK_CLIENT_H */
