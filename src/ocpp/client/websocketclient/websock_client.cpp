#include "websock_client.h"
#include "utils/utils.h"

#include <functional>
#include <iostream>

const struct lws_protocols ocpp1_6::client::WebSocketClient::m_protocols[] =
{
    {"WebSocketClient", &ocpp1_6::client::WebSocketClient::eventcb, sizeof(ocpp1_6::client::WebSocketClient *), 0, 0, nullptr, 0},
    {nullptr, nullptr, 0, 0, 0, nullptr, 0}
};

int32_t web_socket_client_test(void)
{
    class TestListener : public ocpp1_6::client::WebSocketClient::IListener
    {
    public:
        void wsClientConnected() override
        {
            d_log("WebSocket connected!");
        }

        void wsClientDisconnected() override
        {
            d_log("WebSocket disconnected.");
        }
        void wsClientFailed() override
        {
            e_log("WebSocket connection failed!");
        }

        void wsClientDataReceived(const void *data, size_t len) override
        {
            std::string msg(static_cast<const char *>(data), len);
            d_log("Received: %s", msg.c_str());
        }

        void wsClientError() override
        {
            e_log("WebSocket client error occurred!");
        }
    };

    ocpp1_6::client::WebSocketClient client;
    TestListener listener;
    client.registerListener(&listener);
    // std::string url = "ws://172.30.1.55:8180/steve/websocket/ChargeBox1";
    std::string url = "ws://192.168.18.128:8080/steve/websocket/CentralSystemService/ChargeBox1";
    std::string protocol = "ocpp1.6";
    ocpp1_6::auth::Credentials credentials;
    credentials.user = "ChargeBox1";
    credentials.password = "1234";
    std::chrono::milliseconds connect_timeout{3000};
    bool success = client.Connect(url, protocol, credentials, connect_timeout);
    if (!success)
    {
        e_log("Failed to initiate connection.");
        return -1;
    }

    while (1)
    {
        for (int i = 0; i < 10; i++)
        {
            sleep(10);
            client.disConnect();
            sleep(10);
            client.Connect(url, protocol, credentials, connect_timeout);
        }
    }
    return 0;
}

static int lws_http_basic_auth_gen2_ocpp(const char *user, const char *pw, size_t pw_len, char *buf, size_t len)
{
    size_t n = strlen(user), m = pw_len;
    char b[128];
    if (len < 6 + ((4 * (n + m + 1)) / 3) + 1)
    {
        return 1;
    }

    (void)memcpy(buf, "Basic ", 6);

    n = (unsigned int)lws_snprintf(b, sizeof(b), "%s:", user);
    if ((n + pw_len) >= sizeof(b) - 2)
    {
        return 2;
    }
    (void)memcpy(&b[n], pw, pw_len);

    n += pw_len;

    (void)lws_b64_encode_string(b, (int)n, buf + 6, (int)len - 6);
    buf[len - 1] = '\0';
    return 0;
}

ocpp1_6::client::WebSocketClient::WebSocketClient()
    : m_listener(nullptr),
      m_thread(nullptr),
      m_processEnd(false),
      m_retry_interval_ms(0),
      m_ping_interval_s(0),
      m_connection_error_notified(false),
      m_url(),
      m_protocol(""),
      m_credentials(),
      m_connected(false),
      m_context(nullptr),
      m_log_context(),
      m_sched_list(),
      m_wsi(),
      m_retry_policy(),
      m_retry_count(0),
      m_queue(),
      m_fragmented_frame(nullptr),
      m_fragmented_frame_size(0),
      m_fragmented_frame_index(0)
{
    // d_log("websocketclient() client");
    m_retry_policy =
    {
        .retry_ms_table = m_retry_delay_table.data(),
        .retry_ms_table_count = 4,
        .conceal_count = 0,
        .secs_since_valid_ping = m_ping_interval_s,
        .secs_since_valid_hangup = static_cast<uint16_t>(2U * m_ping_interval_s),
        .jitter_percent = 20
    };
}

ocpp1_6::client::WebSocketClient::~WebSocketClient()
{
    d_log("~Websocketclient()");
    disConnect();
    releaseFragmentedFrame();
}

bool ocpp1_6::client::WebSocketClient::Connect(const std::string &url,
                                               const std::string &protocol,
                                               const auth::Credentials &credentials,
                                               std::chrono::milliseconds connect_timeout,
                                               std::chrono::milliseconds retry_interval,
                                               std::chrono::milliseconds ping_interval)
{
    bool ret = false;
    // d_log("websocketclient::connect()");

    if (m_listener == nullptr)
    {
        e_log("WebSocketClient::Connect: listener not registered!");
        return false;
    }
    else if (m_thread != nullptr)
    {
        e_log("WebSocketClient::Connect: already connected!");
        return false;
    }
    else
    {
        d_log("url:%s", url.c_str());
        m_url = url;
        if (m_url.isValid() && (("ws" == m_url.getProtocol()) || ("wss" == m_url.getProtocol())))
        {
            // memset(&m_log_context, 0, sizeof(m_log_context));
            // m_log_context.u.emit = lwsl_emit_stderr; /* lwsl_emit_stderr 是 LWS 内置的日志输出函数，它会把日志打印到标准错误（stderr）。 */
            // m_log_context.u.emit = my_log_emit; /* 替换日志 */
            // m_log_context.lll_flags = (LLL_ERR | LLL_WARN | LLL_NOTICE);

            struct lws_context_creation_info info;
            memset(&info, 0, sizeof(info));
            info.options = CONTEXT_PORT_NO_LISTEN;
            info.port = CONTEXT_PORT_NO_LISTEN; // 客户端不需要监听
            info.protocols = m_protocols;
            info.timeout_secs = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>(connect_timeout).count());
            info.connect_timeout_secs = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>(connect_timeout).count());

            m_credentials = credentials;

            if ("wss" == m_url.getProtocol())
            {
                d_log("setupSSLInfo");
                setupSSLInfo(&info);
            }

            m_context = lws_create_context(&info);
            if (nullptr != m_context)
            {
                m_processEnd = false;
                m_connection_error_notified = false;
                m_connected = false;
                m_retry_count = 0;
                m_retry_interval_ms = static_cast<uint32_t>(retry_interval.count());
                m_ping_interval_s = static_cast<uint16_t>(std::chrono::duration_cast<std::chrono::seconds>(ping_interval).count());
                m_protocol = protocol;
                memset(&m_sched_list, 0, sizeof(m_sched_list));
                m_thread = new std::thread(std::bind(&WebSocketClient::process, this));       /* 启动处理线程 */
                lws_sul_schedule(m_context, 0, &m_sched_list, WebSocketClient::connectcb, 1); /* 首次秒连，连接失败后5s重连 添加延迟连接 加 重连机制 1000 µs = 1ms*/
                ret = true;
            }
        }
        return ret;
    }
}

bool ocpp1_6::client::WebSocketClient::disConnect()
{
    bool ret = false;

    if (nullptr != m_thread)
    {
        m_processEnd = true;
        m_retry_count = 0; /* 主动断开 清零重连计数器 */
        SendMsg *msg;
        while (m_queue.pop(msg, 0))
        {
            if (nullptr != msg)
            {
                delete msg;
            }
        }

        lws_cancel_service(m_context);

        if (std::this_thread::get_id() != m_thread->get_id())
        {
            m_thread->join();
        }
        else
        {
            m_thread->detach();
        }

        delete m_thread;
        m_thread = nullptr;
        m_connected = false;
        ret = true;
    }
    return ret;
}

bool ocpp1_6::client::WebSocketClient::isConnect()
{
    return m_connected;
}

bool ocpp1_6::client::WebSocketClient::send(const void *data, uint64_t size)
{
    bool ret = false;
    if (m_connected)
    {
        SendMsg *msg = new SendMsg(data, size);
        ret = m_queue.push(msg);
        lws_cancel_service(m_context); /* 请立刻停止等待，我要关了 */
    }
    return ret;
}

void ocpp1_6::client::WebSocketClient::registerListener(IListener *listener)
{
    if (m_listener)
    {
        delete m_listener;
    }
    m_listener = listener;
}

void ocpp1_6::client::WebSocketClient::process()
{
    d_log("websocket client process start");
    pthread_setname_np(pthread_self(), "WebSocketClient_process");
    int service_ret = 0;
    while ((false == m_processEnd))
    {
        service_ret = lws_service(m_context, 0);
        if (0 > service_ret)
        {
            e_log("lws_service() returned error: %d. Terminating WebSocket context.", service_ret);
            // if (m_listener && !m_connection_error_notified)
            // {
            //     m_connection_error_notified = true;
            //     m_listener->wsClientError();
            // }
            break;
        }
        // 如果 lws_service 返回 0，可短暂休眠避免 CPU 占用过高
        // if (service_ret == 0)
        // {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // }
    }
    d_log("WebSocketClient::process exiting...");
    if (!m_processEnd)
    {
        e_log("WebSocketClient::process: disconnect");
        disConnect();
        m_listener->wsClientError();
    }
    if (nullptr != m_context)
    {
        lws_context_destroy(m_context);
    }
    d_log("WebSocketClient::process end");
}

void ocpp1_6::client::WebSocketClient::beginFragmentedFrame(size_t frame_size)
{
    // 释放当前正在进行的分片帧，以确保内存的清洁和重用
    releaseFragmentedFrame();

    // 分配空间用于存储分片帧的数据
    m_fragmented_frame = new uint8_t[frame_size];
    m_fragmented_frame_size = frame_size;
}

void ocpp1_6::client::WebSocketClient::appendFragmentedDate(const void *data, size_t size)
{
    // 计算本次实际复制的数据长度
    size_t copy_len = size;
    // 如果追加本次数据后超出缓冲区大小，则只复制剩余空间的数据
    if ((m_fragmented_frame_index + size) >= m_fragmented_frame_size)
    {
        copy_len = m_fragmented_frame_size - m_fragmented_frame_index;
    }
    // 将数据复制到分片缓冲区中的当前位置
    memcpy(&m_fragmented_frame[m_fragmented_frame_index], data, copy_len);
    // 更新分片缓冲区中的数据量计数
    m_fragmented_frame_index += copy_len;
}

void ocpp1_6::client::WebSocketClient::releaseFragmentedFrame()
{
    // 释放分片帧占用的内存
    delete[] m_fragmented_frame;
    // 将分片帧指针置空，以避免悬空指针问题
    m_fragmented_frame = nullptr;
    // 重置分片帧大小为0，表示当前没有分片帧数据
    m_fragmented_frame_size = 0;
    // 重置分片帧索引为0，准备接收新的分片帧数据
    m_fragmented_frame_index = 0;
}

void ocpp1_6::client::WebSocketClient::connectcb(struct lws_sorted_usec_list *sul) noexcept
{
    WebSocketClient *self_client = reinterpret_cast<WebSocketClient *>(reinterpret_cast<char *>(sul) - offsetof(WebSocketClient, m_sched_list)); /* 获取本实例地址 */

    if ((nullptr == self_client) || (true == self_client->m_processEnd))
    {
        d_log("error self_client:%p or self_client process end:%d", self_client, self_client->m_processEnd);
        return;
    }

    // 初始化客户端连接信息结构体
    struct lws_client_connect_info i;
    memset(&i, 0, sizeof(i));
    i.context = self_client->m_context;
    i.address = self_client->m_url.getHost().c_str();
    i.path = self_client->m_url.getPath().c_str();
    i.host = i.address;
    i.origin = i.address;

    // 根据URL协议设置SSL连接选项
    if (self_client->m_url.getProtocol() == "wss")
    {
        i.ssl_connection = LCCSCF_USE_SSL;
        if (self_client->m_credentials.allow_selfsigned_certificates)
        {
            i.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED;
        }
        if (self_client->m_credentials.allow_expired_certificates)
        {
            i.ssl_connection |= LCCSCF_ALLOW_EXPIRED;
        }
        if (self_client->m_credentials.accept_untrusted_certificates)
        {
            i.ssl_connection |= LCCSCF_ALLOW_INSECURE;
        }
        if (self_client->m_credentials.skip_server_name_check)
        {
            i.ssl_connection |= LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        }
        i.port = 443;
    }
    else
    {
        i.port = 8080;
    }

    // 如果URL中指定了端口，则使用指定的端口
    if (self_client->m_url.getPort())
    {
        i.port = static_cast<int>(self_client->m_url.getPort());
    }

    // 设置协议、本地协议名称、websocket实例指针、重试策略和用户数据
    i.protocol = self_client->m_protocol.c_str();
    i.local_protocol_name = "WebSocketClient";
    i.retry_and_idle_policy = &self_client->m_retry_policy;
    i.pwsi = &self_client->m_wsi; /* 用于同步获取 wsi 句柄 输出：保存 wsi 到成员 */
    i.userdata = self_client;     /* 传递本实例, 之后可以在eventcb中使用本实例的成员变量 用于 eventcb 中获取 this 输入：绑定 this 到 wsi*/

    // 尝试建立websocket连接，如果失败则根据重试策略重新安排连接尝试
    if (nullptr == lws_client_connect_via_info(&i))
    {
        d_log("Scheduling reconnect, attempt %d", self_client->m_retry_count);
        lws_retry_sul_schedule(self_client->m_context, 0, sul, &self_client->m_retry_policy, &WebSocketClient::connectcb, &self_client->m_retry_count); /* 重连机制 */
    }
}

int ocpp1_6::client::WebSocketClient::eventcb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) noexcept
{
    int ret = 0;
    bool retry = false;

    // 处理不同的websocket事件
    switch (reason)
    {
    // 取消事件等待回调
    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
    {
        if ((nullptr != wsi) && (nullptr != user))
        {

            WebSocketClient *self_client = reinterpret_cast<WebSocketClient *>(user);
            // d_log("WebSocketClient: Event wait cancelled");
            if (!self_client->m_processEnd && !self_client->m_queue.empty())
            {
                lws_callback_on_writable(self_client->m_wsi);
            }
        }
        break;
    }

    // 连接错误回调
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
        e_log("WebSocketClient: Connection error");
        /* 只在 user != nullptr 时通知 listener 和设置 retry */
        if (nullptr != user)
        {
            WebSocketClient *self_client = reinterpret_cast<WebSocketClient *>(user);
            if (false == self_client->m_connection_error_notified)
            {
                self_client->m_connection_error_notified = true;
                // e_log("WebSocketClient: Connection error");
                self_client->m_listener->wsClientFailed();
            }
            // 判断是否需要重连
            if (self_client->m_retry_interval_ms != 0)
            {
                retry = true;
            }
        }
        else
        {
            e_log("WebSocketClient: Connection error (user is nullptr, likely immediate TCP reject)");
        }
        break;
    }
    default:
    {
        if (nullptr == user)
        {
            return 0;
        }

        WebSocketClient* self_client = static_cast<WebSocketClient*>(user);
        switch (reason)
        {
        // 添加握手头回调
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        {
            d_log("WebSocketClient: Adding HTTP headers");
            unsigned char **p = (unsigned char **)in, *end = (*p) + len;
            char out[128];

            if (self_client->m_credentials.user.empty())
            {
                d_log("WebSocketClient: No user name specified");
                break;
            }

            if (lws_http_basic_auth_gen2_ocpp(self_client->m_credentials.user.c_str(),
                                              self_client->m_credentials.password.c_str(),
                                              self_client->m_credentials.password.size(),
                                              out,
                                              sizeof(out)))
                break;

            out[sizeof(out) - 1] = '\0';
            d_log("Sending Authorization header: origin_user:%s, origin_pw:%s, out:%s", self_client->m_credentials.user.c_str(), self_client->m_credentials.password.c_str(), out);
            if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_AUTHORIZATION, (unsigned char *)out, (int)strlen(out), p, end))
                return -1;

            break;
        }

        // 连接建立回调
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            d_log("WebSocketClient: Connection established");
            self_client->m_retry_count = 0; /* 连接建立成功, 清理重连计数 */
            self_client->m_connected = true;
            self_client->m_listener->wsClientConnected();
            break;
        }

        // 接收数据回调
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            // d_log("WebSocketClient: Received data");
            if (self_client->m_listener)
            {
                bool is_first = (lws_is_first_fragment(wsi) == 1);
                bool is_last = (lws_is_final_fragment(wsi) == 1);
                size_t remaining_length = lws_remaining_packet_payload(wsi);
                // 处理完整帧和分片帧
                if (is_first && is_last)
                {
                    self_client->m_listener->wsClientDataReceived(in, len);
                }
                else if (is_first)
                {
                    self_client->beginFragmentedFrame(len + remaining_length);
                    self_client->appendFragmentedDate(in, len);
                }
                else
                {
                    self_client->appendFragmentedDate(in, len);
                    if (is_last)
                    {
                        self_client->m_listener->wsClientDataReceived(self_client->m_fragmented_frame, self_client->m_fragmented_frame_size);
                        self_client->releaseFragmentedFrame();
                    }
                }
            }
            break;
        }

        // 可写事件回调
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            // d_log("WebSocketClient: Writeable");
            bool error = false;
            SendMsg *msg = nullptr;
            // 发送消息队列中的消息
            while (self_client->m_queue.pop(msg, 0) && !error)
            {
                if (lws_write(wsi, msg->payload, msg->size, LWS_WRITE_TEXT) < static_cast<int>(msg->size))
                {
                    error = true;
                }
                if (msg != nullptr)
                {
                    delete msg;
                }
            }
            if (error)
            {
                return -1;
            }
            break;
        }

        // 客户端HTTP连接关闭回调
        case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
        {  d_log("WebSocketClient: HTTP connection closed");
            if (self_client->m_retry_interval_ms != 0)
            {
                retry = true;
            }
            break;
        }
        // 客户端关闭回调
        case LWS_CALLBACK_CLIENT_CLOSED:
        {

            d_log("WebSocketClient: self_client closed");
            self_client->m_connected = false;
            self_client->m_listener->wsClientDisconnected();
            // 判断是否需要重连
            if (self_client->m_retry_interval_ms != 0)
            {
                retry = true;
            }
            // 清空消息队列
            SendMsg *msg;
            while (self_client->m_queue.pop(msg, 0))
            {
                if (msg != nullptr)
                {
                    delete msg;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    }

    // 处理重连逻辑
    if ((true == retry) && (nullptr != user))
    {
        WebSocketClient* self_client = static_cast<WebSocketClient*>(user);
        if (nullptr != wsi)
        {
            lws_retry_sul_schedule_retry_wsi(wsi, &self_client->m_sched_list, &WebSocketClient::connectcb, &self_client->m_retry_count);
        }
        else
        {
            lws_sul_schedule(self_client->m_context, 0, &self_client->m_sched_list, WebSocketClient::connectcb, 1);
        }
    }
    else
    {
        ret = lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    return ret;
}

// 配置SSL信息的辅助函数
void ocpp1_6::client::WebSocketClient::setupSSLInfo(struct lws_context_creation_info *info)
{
    if (!m_credentials.tls12_cipher_list.empty())
    {
        info->client_ssl_cipher_list = m_credentials.tls12_cipher_list.c_str();
    }
    if (!m_credentials.tls13_cipher_list.empty())
    {
        info->client_tls_1_3_plus_cipher_list = m_credentials.tls13_cipher_list.c_str();
    }
    if (m_credentials.encoded_pem_certificates)
    {
        if (!m_credentials.server_certificate_ca.empty())
        {
            info->client_ssl_ca_mem = m_credentials.server_certificate_ca.c_str();
            info->client_ssl_ca_mem_len = static_cast<unsigned int>(m_credentials.server_certificate_ca.size());
        }
        if (!m_credentials.client_certificate.empty())
        {
            info->client_ssl_cert_mem = m_credentials.client_certificate.c_str();
            info->client_ssl_cert_mem_len = static_cast<unsigned int>(m_credentials.client_certificate.size());
        }
        if (!m_credentials.client_certificate_private_key.empty())
        {
            info->client_ssl_key_mem = m_credentials.client_certificate_private_key.c_str();
            info->client_ssl_key_mem_len = static_cast<unsigned int>(m_credentials.client_certificate_private_key.size());
        }
    }
    else
    {
        if (!m_credentials.server_certificate_ca.empty())
        {
            info->client_ssl_ca_filepath = m_credentials.server_certificate_ca.c_str();
        }
        if (!m_credentials.client_certificate.empty())
        {
            info->client_ssl_cert_filepath = m_credentials.client_certificate.c_str();
        }
        if (!m_credentials.client_certificate_private_key.empty())
        {
            info->client_ssl_private_key_filepath = m_credentials.client_certificate_private_key.c_str();
        }
    }
    if (!m_credentials.client_certificate_private_key_passphrase.empty())
    {
        info->client_ssl_private_key_password = m_credentials.client_certificate_private_key_passphrase.c_str();
    }
}
