#ifndef WEBSOCK_CLIENT
#define WEBSOCK_CLIENT


#include "websocket/libwebsockets.h"
#include "ocpp/credentials/credentials.h"
#include "tool/url/Url.h"
#include "ocpp/credentials/credentials.h"
#include "ocpp/common/Queue.h"

#include <stdint.h>
#include <string>
#include <chrono>
#include <thread>

namespace ocpp1_6
{
    namespace client
    {
        class WebSocketClient
        {
        public:
            WebSocketClient();
            ~WebSocketClient();

            bool Connect(const std::string& url,
                         const std::string& protocol,
                         const auth::Credentials& credentials,
                         std::chrono::milliseconds connect_timeout,
                         std::chrono::milliseconds retry_interval = std::chrono::milliseconds(5000),
                         std::chrono::milliseconds ping_interval = std::chrono::milliseconds(5000));

            bool disConnect();
            bool isConnect();
            bool send(const void* data, uint64_t size);
            bool recv();

            class IListener;
            void registerListener(IListener *listener);

            class IListener
            {
            public:
                virtual ~IListener() {}

                virtual void wsClientConnected() = 0;
                virtual void wsClientFailed() = 0;
                virtual void wsClientDisconnected() = 0;
                virtual void wsClientError() = 0;
                virtual void wsClientDataReceived(const void *data, uint64_t size) = 0;
            };
        private:
            struct SendMsg
            {
                uint64_t size;
                unsigned char *data;
                unsigned char *payload;
                SendMsg(const void *_data, uint64_t _size):size(_size)
                {
                    data = new unsigned char[LWS_PRE + _size];
                    (void)memcpy(&data[LWS_PRE], _data, _size);
                    payload = &data[LWS_PRE];
                }
            };

            IListener* m_listener;
            std::thread *m_thread;
            bool m_end;

            uint32_t m_retry_interval_ms;
            uint16_t m_ping_interval_s;

            bool m_connection_error_notified;

            Url m_url;

            std::string m_protocol;
            auth::Credentials m_credentials;

            bool m_connected;
            struct lws_context* m_context;
            lws_log_cx_t m_log_context;
            lws_sorted_usec_list_t m_sched_list;

            struct lws* m_wsi;
            lws_retry_bo m_retry_policy;
            uint16_t m_retry_count;

            ocpp::common::Queue<SendMsg*> m_queue;

            uint8_t* m_fragmented_frame;
            size_t m_fragmented_frame_size;
            size_t m_fragmented_frame_index;

            void setupSSLInfo(struct lws_context_creation_info *info);

            void process();

            void beginFragmentedFrame(size_t frame_size);
            void appendFragmentedDate(const void* data, size_t size);
            void releaseFragmentedFrame();

            static void connectcb(struct lws_sorted_usec_list* sul) noexcept;
            static int eventcb(struct lws* wsi, enum lws_callback_reasons reasons, void *user, void *in, size_t len) noexcept;
        };
    } // namespace client

} // namespace ocpp1_6


int32_t web_socket_client_test(void);

#endif /* WEBSOCK_CLIENT */
