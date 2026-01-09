
#ifndef OCPPCLIENT_H
#define OCPPCLIENT_H

#include <string>
#include <functional>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "credentials.h"
#include "ocpp/client/websocketclient/websock_client.h"
#include "ocpp/tool/threadpool/ThreadPool.h"
#include "ocpp/tool/jsonValidator/JsonValidator.h"
#include "ocpp/config/ConfigManager.h"

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
            unsigned int messageTimeout;
            unsigned int transactionMessageAttempts;
            Credentials credentials;
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

            // 发送OCPP call消息, 需要等待响应或超时
            virtual bool sendCall(const std::string &action, const rapidjson::Document &reqPayload, rapidjson::Document &rspPayload, unsigned int connectorId=0, bool isTx=false, int timeout=5000) = 0;

            // 发送OCPP error消息
            virtual bool sendCallError(const std::string& uniqueId, const std::string& errorCode, const std::string& errMsg, const rapidjson::Value& errorDetails, unsigned int connectorId=0, bool isTx=false, int timeout=3000) = 0;

            // 发送OCPP result消息
            virtual bool sendCallResult(const std::string& uniqueId, const std::string& action, const rapidjson::Document& payload, unsigned int connectorId=0, bool isTx=false, int timeout=3000) = 0;

            // 注册CP Call消息响应的处理函数
            virtual void registerCallHandler(const std::string& action, std::function<void(rapidjson::Document&)> handler) = 0;

            virtual void registerCallResultHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler) = 0;

            // 设置连接过程状态变化的监听器
            virtual void setOCPPClientListener(IOCPPClientListener *listener) = 0;
        };

        class OCPPClient : public IOCPPClient, public WebSocketClient::IListener
        {
        public:
            OCPPClient(WebSocketClient& webSocketClient, JsonValidator& validator, unsigned int connectorNum);
            ~OCPPClient();

            /* IOCPPClient重写 */
            bool start() override;
            bool stop() override;
            bool disConnect() override;
            bool isConnected() const override;
            // 消息发送

            // 同步：超时等待
            /**
             * @brief 同步发送消息 等到回复或超时
             *
             * @param connectorId   枪编号：0代表桩，1-n代表枪编号
             * @param action        命令
             * @param reqPayload    请求参数
             * @param rspPayload    返回参数
             * @param timeout       超时时间
             * @param isTx          是否是交易相关消息 (交易相关消息是有重发机制的)
             *
             */
            bool sendCall(const std::string &action,
                          const rapidjson::Document &reqPayload,
                          rapidjson::Document &rspPayload,
                          unsigned int connectorId,
                          bool isTx,
                          int timeout) override;

            bool sendCallError(const std::string&      uniqueId,
                       const std::string&      errorCode,
                       const std::string&      errorDescription,
                       const rapidjson::Value& errorDetails,
                       unsigned int            connectorId,
                       bool                    isTx,
                       int                     timeout) override;

            bool sendCallResult(const std::string&         uniqueId,
                        const std::string&         action,
                        const rapidjson::Document& payload,
                        unsigned int               connectorId,
                        bool                       isTx,
                        int                        timeout) override;
            // 响应回调, 不要再handler操作全局数据
            void registerCallHandler(const std::string& action, std::function<void(rapidjson::Document&)> handler) override;

            // 监听回调
            void setOCPPClientListener(IOCPPClientListener* listener) override;

            /* WebSocketClient IListener 重写 */
            void wsClientConnected() override;
            void wsClientFailed() override;
            void wsClientDisconnected() override;
            void wsClientError() override;

            /* ocpp收到server通过websocket的回复 */
            void wsClientDataReceived(const void* data, size_t size) override;

            /*  */
        private:
            struct PendingCall /* 客户端发送请求后，在收到回复前，该请求处于 pending（挂起）状态 一个 Call 发送后等待 CallResult 或 CallError */
            {
                unsigned int connId;
                std::string uniqueId;
                bool isTx;
                std::string raw;
                int retryCount = 0;
                int maxRetry = 0;
                int retryIntervalMs = 0;
                std::chrono::steady_clock::time_point expireAt;
                std::chrono::steady_clock::time_point nextRetry;
                bool finished = false; /* Call 收到回应 */
                bool success = false; /* 是否是CallResult */
                std::string response;

                std::mutex mtx;
                std::condition_variable cv;
            };

            struct ConnectorContext
            {
                common::Queue<std::string> txQueue;
                common::Queue<std::string> nonTxQueue;
                std::mutex mtx;
                std::condition_variable cv;
                bool running = true;
            };
            std::vector<std::unique_ptr<ConnectorContext>> m_connectors;
            std::vector<std::thread> m_sendThreads;

            std::unordered_map<std::string, std::shared_ptr<PendingCall>> m_pending;
            std::mutex m_pendingMutex;

            std::thread m_retryThread;

            std::unordered_map<std::string, std::function<void(rapidjson::Document &)>> m_callHandlers; /* server端发来的Call函数 */
            std::mutex m_callHandlerMtx;

            ConnectInfo m_connectInfo;
            WebSocketClient &m_ws;
            JsonValidator &m_validator;

            std::atomic_bool m_isConnected{false};
            std::atomic_bool m_running{false};

            IOCPPClientListener *m_listener{nullptr};
            unsigned m_connectorNum;
            ThreadPool m_threadPool;
            bool doConnect();
            bool doSend(const std::string &msg);

            /**
             * @brief 解析接收的server端信息
             *
             * @param msg 解析server端 发送的消息
             */
            void parseMessage(const std::string &msg);
            void sendThread(unsigned int connectorId);
            void retryTimeoutThread();
            bool initConnectInfo();
        };

    } /* client */
} /* ocpp1_6 */

#endif /* OCPPCLIENT_H */
