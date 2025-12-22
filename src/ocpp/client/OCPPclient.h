
#ifndef OCPPCLIENT_H
#define OCPPCLIENT_H

#include <string>
#include <functional>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "ocpp/credentials/credentials.h"
#include "ocpp/client/websocketclient/websock_client.h"
#include "ocpp/tool/threadpool/ThreadPool.h"
#include "ocpp/config/configManager.h"

namespace ocpp1_6
{
    namespace client
    {
        struct ConnectInfo
        {
            int retryInterval;
            int webSocketPingInterval;
            std::string url;
            std::string user;
            std::string password;
            std::string protocol;
            unsigned int timeout;
            auth::Credentials credentials;
        };

        class IOCPPClient
        {
        public:
            class IOCPPClientListener
            {
            public:
                virtual void onConnected() = 0;
                virtual void onFailed() = 0;
                virtual void onDisconnected() = 0;
                virtual void onError() = 0;
            };

            virtual ~IOCPPClient() = default;

            virtual bool start() = 0;
            virtual bool stop() = 0;
            virtual bool disConnect() = 0;
            virtual bool isConnected() const = 0;

            virtual bool sendCall(const std::string &action, const rapidjson::Document &payload) = 0;

            // 发送OCPP error消息
            virtual bool sendCallError(const std::string &uniqueId, const std::string &errorCode, const std::string &errMsg) = 0;

            // 发送OCPP result消息
            virtual bool sendCallResult(const std::string &uniqueId, const rapidjson::Document &payload) = 0;

            // 注册CP Call消息响应的处理函数
            virtual void registerCallHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) = 0;

            virtual void registerCallResultHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) = 0;

            // 注册CS Call消息处理函数（call_result and call_error）
            virtual void registerCallErrorHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) = 0;

            // 设置连接过程状态变化的监听器
            virtual void setOCPPClientListener(IOCPPClientListener *listener) = 0;
        };

        class OCPPClient : public IOCPPClient, public WebSocketClient::IListener
        {
        public:
            OCPPClient(WebSocketClient& webSocketClient, ConfigManager& configManager, ThreadPool& threadPool);
            ~OCPPClient();

            /* IOCPPClient重写 */
            bool start() override;
            bool stop() override;
            bool disConnect() override;
            bool isConnected() const override;
            bool sendCall(const std::string &action, const rapidjson::Document &payload) override;
            bool sendCallError(const std::string &uniqueId, const std::string &errorCode, const std::string &errMsg) override;
            bool sendCallResult(const std::string &uniqueId, const rapidjson::Document &payload) override;
            void registerCallHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) override;
            void registerCallResultHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) override;
            void registerCallErrorHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) override;
            void setOCPPClientListener(IOCPPClientListener *listener) override;

            /* WebSocketClient IListener 重写 */
            void wsClientConnected() override;
            void wsClientFailed() override;
            void wsClientDisconnected() override;
            void wsClientError() override;
            void wsClientDataReceived(const void* data, size_t size) override;

            /*  */
        private:
            std::unordered_map<std::string, std::string> m_pendingRequests;
            // 记录发送时间
            std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_sendTimeMap;
            mutable std::mutex m_pendingRequestsMutex;

            std::condition_variable m_responseCV;
            std::mutex m_responseMutex;
            std::atomic<bool> m_responseReceived;

            WebSocketClient& m_webSocketClient;
            IOCPPClientListener* m_ocppListener;

            std::unordered_map<std::string, std::function<void(rapidjson::Document &)>> m_callHandlers;
            std::unordered_map<std::string, std::function<void(rapidjson::Document &)>> m_callResultHandlers;
            std::unordered_map<std::string, std::function<void(rapidjson::Document &)>> m_callErrorHandlers;
            mutable std::mutex m_handlerMutex;

            std::atomic<uint32_t> m_uniqueIdCounter;
            std::atomic<bool> m_isConnected; // OCPPClient是否连接
            std::atomic<bool> m_isRunning; // 线程运行状态

            std::queue<std::string> m_sendQueue;
            std::queue<std::string> m_recvQueue;
            std::thread m_sendThread;
            std::thread m_recvThread;
            std::mutex m_sendMutex;
            std::mutex m_recvMutex;
            std::condition_variable m_sendCV;
            std::condition_variable m_recvCV;

            std::atomic<bool> m_hasMsg;
            std::mutex m_mutex;
            std::condition_variable m_condVar;
            ConnectInfo m_connectInfo;
            ConfigManager &m_configManager;
            ThreadPool &m_threadPool;

            bool initConnectInfo();
            bool doConnect();
            bool doSend(const std::string payload);
            void sendThreadFunc();
            void recvThreadFunc();
            void parseMessage(const std::string& message);
        };

    } /* client */
} /* ocpp1_6 */

#endif /* OCPPCLIENT_H */
