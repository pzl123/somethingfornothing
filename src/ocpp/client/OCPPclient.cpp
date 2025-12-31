#include <thread>

#include "OCPPclient.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "utils/utils.h"


namespace ocpp1_6
{
    namespace client
    {
        OCPPClient::OCPPClient(ocpp1_6::client::WebSocketClient &webSocketClient, ocpp1_6::ConfigManager &configManager, ThreadPool &threadPool)
            : m_responseReceived(false),
              m_webSocketClient(webSocketClient),
              m_ocppListener(nullptr),
              m_uniqueIdCounter(0),
              m_isConnected(false),
              m_isRunning(false),
              m_hasMsg(false),
              m_connectInfo{60000, 60000,
                            "default_url",
                            "default_user",
                            "default_password",
                            "default_protocol",
                            10000,
                            auth::Credentials()},
              m_configManager(configManager),
              m_threadPool(threadPool)
        {
            m_webSocketClient.registerListener(this);
        }

        OCPPClient::~OCPPClient()
        {
            stop();
        }

        bool OCPPClient::initConnectInfo()
        {
            rapidjson::Document doc;
            if (false == m_configManager.getConfig("", doc))
            {
                e_log("get config error");
                return false;
            }

            if ((false == doc.HasMember("ChargePoint")) && (false == doc.HasMember("OCPP1_6")))
            {
                e_log("get config error");
                return false;
            }

            auto validataConfig = [](rapidjson::Document &config) -> bool
            {
                return config.HasMember("ChargePoint") && config.HasMember("OCPP1_6") &&
                       config["ChargePoint"].IsObject() && config["OCPP1_6"].IsObject() &&
                       config["ChargePoint"].HasMember("Protocol") &&
                       config["ChargePoint"].HasMember("ConnectionTimeout") &&
                       config["ChargePoint"].HasMember("ConnectionUrl") &&
                       config["ChargePoint"].HasMember("ChargePointIdentifier") &&
                       config["ChargePoint"].HasMember("SecurityProfile") &&
                       config["OCPP1_6"].HasMember("AuthorizationKey") &&
                       config["ChargePoint"].HasMember("RetryInterval") &&
                       config["OCPP1_6"].HasMember("WebSocketPingInterval");
            };
            if (false == validataConfig(doc));
            {
                e_log("Config verification failed");
                return false;
            }

            rapidjson::Value jChargePoint;
            jChargePoint.CopyFrom(doc, doc.GetAllocator());
            rapidjson::Value jOCPP1_6;
            jOCPP1_6.CopyFrom(doc, doc.GetAllocator());

            auto configDebugInfo = [](const char* name, rapidjson::Value &value)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                value.Accept(writer);
                const char *output = buffer.GetString();
                d_log("%s config:%s", name, output);
            };

            configDebugInfo("ChargePoint", jChargePoint);
            configDebugInfo("Ocpp", jOCPP1_6);

            auth::Credentials credentials;
            unsigned int security_profile = jChargePoint["SecurityProfile"].GetUint();
            std::string authorization_key = jOCPP1_6["AuthorizationKey"].GetString();
            std::string url = jChargePoint["ConnectionUrl"].GetString();

            if (url.back() != '/')
            {
                url += "/";
            }
            url += Url::encode(jChargePoint["ChargePointIdentifier"].GetString());

            if (!authorization_key.empty() && (security_profile <= 2))
            {
                credentials.user = jChargePoint["ChargePointIdentifier"].GetString();
                credentials.password = authorization_key;
            }

            d_log("url:%s credentials.user:%s, credentials.password:%s", url.c_str(), credentials.user.c_str(), credentials.password.c_str());

            // TLS 配置项检查
            auto hasTlsConfig = [&]() -> bool
            {
                return jChargePoint.HasMember("Tlsv12CipherList") &&
                       jChargePoint.HasMember("Tlsv13CipherList") &&
                       jChargePoint.HasMember("InternalCertificateManagementEnabled") &&
                       jChargePoint.HasMember("TlsServerCertificateCa") &&
                       jChargePoint.HasMember("TlsClientCertificate") &&
                       jChargePoint.HasMember("TlsClientCertificatePrivateKey") &&
                       jChargePoint.HasMember("TlsClientCertificatePrivateKeyPassphrase") &&
                       jChargePoint.HasMember("TlsAllowSelfSignedCertificates") &&
                       jChargePoint.HasMember("TlsAllowExpiredCertificates") &&
                       jChargePoint.HasMember("TlsAcceptNonTrustedCertificates")
                       && jChargePoint.HasMember("TlsSkipServerNameCheck");
            };
            if (false == hasTlsConfig())
            {
                e_log("Failed to load TLS configuration");
                return false;
            }

            if (security_profile != 1)
            {
                credentials.tls12_cipher_list = jChargePoint["Tlsv12CipherList"].GetString();
                credentials.tls13_cipher_list = jChargePoint["Tlsv13CipherList"].GetString();
                bool internalCertMgmt = jChargePoint["InternalCertificateManagementEnabled"].GetBool();

                /* 外部管理证书 */
                if ((true == internalCertMgmt) || (security_profile == 0))
                {
                    credentials.server_certificate_ca = jChargePoint["TlsServerCertificateCa"].GetString();
                    if ((security_profile == 0) || (security_profile == 3)) // 排除security_profile=2(单项认证)
                    {
                        credentials.client_certificate = jChargePoint["TlsClientCertificate"].GetString();
                        credentials.client_certificate_private_key = jChargePoint["TlsClientCertificatePrivateKey"].GetString();
                        credentials.client_certificate_private_key_passphrase = jChargePoint["TlsClientCertificatePrivateKeyPassphrase"].GetString();
                    }
                    credentials.allow_selfsigned_certificates = jChargePoint["TlsAllowSelfSignedCertificates"].GetBool();
                    credentials.allow_expired_certificates = jChargePoint["TlsAllowExpiredCertificates"].GetBool();
                    credentials.accept_untrusted_certificates = jChargePoint["TlsAcceptNonTrustedCertificates"].GetBool();
                    credentials.skip_server_name_check = jChargePoint["TlsSkipServerNameCheck"].GetBool();
                    credentials.encoded_pem_certificates = false;
                }
                else /* 内部管理证书 */
                {
                    credentials.server_certificate_ca = "";
                    if (security_profile == 3)
                    {
                        credentials.client_certificate = "";
                        credentials.client_certificate_private_key = "";
                        credentials.client_certificate_private_key_passphrase = jChargePoint["TlsClientCertificatePrivateKeyPassphrase"].GetString();
                    }
                    credentials.encoded_pem_certificates = true;
                    credentials.allow_selfsigned_certificates = true;
                    credentials.allow_expired_certificates = true;
                    credentials.accept_untrusted_certificates = false;
                    credentials.skip_server_name_check = false;
                }
            }
            m_connectInfo.credentials = credentials;
            m_connectInfo.url = url;
            m_connectInfo.protocol = jChargePoint["Protocol"].GetString();
            m_connectInfo.timeout = jChargePoint["ConnectionTimeout"].GetUint();
            m_connectInfo.retryInterval = jChargePoint["RetryInterval"].GetInt();
            m_connectInfo.webSocketPingInterval = jOCPP1_6["WebSocketPingInterval"].GetInt();

            d_log("m_connectInfo: %s, protocol:%s, timeout:%u, retryInterval:%d, webSocketPingInterval:%d",
                  m_connectInfo.url.c_str(),
                  m_connectInfo.protocol.c_str(),
                  m_connectInfo.timeout,
                  m_connectInfo.retryInterval,
                  m_connectInfo.webSocketPingInterval);
            return true;
        }

        bool OCPPClient::start()
        {
            if (false == initConnectInfo())
            {
                e_log("initConnectInfo error");
                return false;
            }

            if (false == doConnect())
            {
                e_log("OCPPClient connect failed");
                return false;
            }

            if (true == m_isRunning)
            {
                e_log("OCPPClient connected");
                return false;
            }
            m_isRunning = true;

            m_sendThread = std::thread(&OCPPClient::sendThreadFunc, this);
            m_recvThread = std::thread(&OCPPClient::recvThreadFunc, this);
            return true;
        }

        bool OCPPClient::stop()
        {
            m_isRunning = false;
            e_log("ocppclient stop");

            if (true == m_isConnected)
            {
                disConnect();
            }

            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                m_sendCV.notify_all();
            }
            if (m_sendThread.joinable())
            {
                m_sendThread.join();
            }

            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                m_recvCV.notify_all();
            }

            if (m_recvThread.joinable())
            {
                m_recvThread.join();
            }

            d_log("ocppclient stoped");
            return true;
        }

        bool OCPPClient::disConnect()
        {
            return m_webSocketClient.disConnect(true);
        }

        bool OCPPClient::isConnected() const
        {
            return m_isConnected;
        }

        /* OCPP 协议的消息格式就是一个 JSON 数组
        msgTyep msgid     action             payload
        [2, "192837465", "BootNotification", { ... }] */
        bool OCPPClient::sendCall(const std::string &action, const rapidjson::Document &payload)
        {
            d_log("sendCall: action: %s", action.c_str());
            rapidjson::Document msg;
            msg.SetArray();
            rapidjson::Document::AllocatorType& allocator = msg.GetAllocator();

            msg.PushBack(static_cast<int>(MessageType::Call), allocator);
            std::string uniqueId = generateMessageId();
            msg.PushBack(rapidjson::Value(uniqueId.c_str(), allocator).Move(), allocator);
            msg.PushBack(rapidjson::Value(action.c_str(), allocator).Move(), allocator);
            rapidjson::Value payloadCopy;
            payloadCopy.CopyFrom(payload, allocator);
            msg.PushBack(payloadCopy, allocator);

            std::string sendMsg;
            jsonSerialize(sendMsg, msg);

            {
                std::unique_lock<std::mutex> lock(m_pendingRequestsMutex);
                m_pendingRequests[uniqueId] = action;
                m_sendTimeMap[uniqueId] = std::chrono::steady_clock::now();
            }

            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                m_sendQueue.push(sendMsg);
            }
            m_sendCV.notify_one();
            return true;
        }

        bool OCPPClient::sendCallError(const std::string &uniqueId, const std::string &errorCode, const std::string &errMsg)
        {
            if (!isConnected())
            {
                d_log("sendCallError: Connection failed, cannot send call error.");
                return false;
            }

            rapidjson::Document msg;
            msg.SetArray();
            rapidjson::Document::AllocatorType& allocator = msg.GetAllocator();

            msg.PushBack(static_cast<int>(MessageType::Call), allocator);
            msg.PushBack(rapidjson::Value(uniqueId.c_str(), allocator).Move(), allocator);
            msg.PushBack(rapidjson::Value(errorCode.c_str(), allocator).Move(), allocator);
            msg.PushBack(rapidjson::Value(errMsg.c_str(), allocator).Move(), allocator);

            std::string sendMsg;
            jsonSerialize(sendMsg, msg);
            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                m_sendQueue.push(sendMsg);
            }
            m_sendCV.notify_one();
            // return m_webSocketClient.send(sendMsg.c_str(), sendMsg.size());
        }

        bool OCPPClient::sendCallResult(const std::string &uniqueId, const rapidjson::Document &payload)
        {
            if (!isConnected())
            {
                d_log("sendCallResult: Connection failed, cannot send call result.");
                return false;
            }

            rapidjson::Document msg;
            msg.SetArray();
            rapidjson::Document::AllocatorType& allocator = msg.GetAllocator();

            msg.PushBack(static_cast<int>(MessageType::CallResult), allocator);
            msg.PushBack(rapidjson::Value(uniqueId.c_str(), allocator).Move(), allocator);
            rapidjson::Value payloadCopy;
            payloadCopy.CopyFrom(payload, allocator);
            msg.PushBack(payloadCopy, allocator);
            std::string sendMsg;
            jsonSerialize(sendMsg, msg);
            d_log("Send call result: %s", sendMsg.c_str());

            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                m_sendQueue.push(sendMsg);
            }
            m_sendCV.notify_one();
        }

        void OCPPClient::registerCallHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler)
        {
            // d_log("Registering call handler for action: %s", action.c_str());
            std::unique_lock<std::mutex> lock(m_handlerMutex);
            m_callHandlers[action] = handler;
        }

        void OCPPClient::registerCallResultHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler)
        {
            std::unique_lock<std::mutex> lock(m_handlerMutex);
            m_callResultHandlers[action] = handler;
        }

        void OCPPClient::registerCallErrorHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler)
        {
            std::unique_lock<std::mutex> lock(m_handlerMutex);
            m_callErrorHandlers[action] = handler;
        }

        void OCPPClient::setOCPPClientListener(IOCPPClientListener *listener)
        {
            if (listener != nullptr && m_ocppListener == nullptr)
            {
                d_log("OCPPClientListener has been set.");
                m_ocppListener = listener;
            }
        }

        void OCPPClient::wsClientConnected()
        {
            m_isConnected = true;
            if (nullptr != m_ocppListener)
            {
                m_ocppListener->onConnected();
            }
        }

        void OCPPClient::wsClientFailed()
        {
            m_isConnected = false;
            if (m_ocppListener)
            {
                m_ocppListener->onFailed();
            }
        }

        void OCPPClient::wsClientDisconnected()
        {
            d_log("websocket disconnected");
            m_isConnected = false;
            if (m_ocppListener)
            {
                m_ocppListener->onDisconnected();
            }
        }

        void OCPPClient::wsClientError()
        {
            m_isConnected = false;
            if (m_ocppListener)
            {
                m_ocppListener->onError();
            }
        }

        void OCPPClient::wsClientDataReceived(const void *data, size_t size)
        {
            std::string message(static_cast<const char *>(data), size);
            {
                std::unique_lock<std::mutex> lock(m_recvMutex);
                m_recvQueue.push(message);
            }
            m_recvCV.notify_one();
        }

        bool OCPPClient::doConnect()
        {
            if (true == m_isConnected)
            {
                w_log("already connected");
                return true;
            }

            if (false == m_webSocketClient.Connect(m_connectInfo.url, m_connectInfo.protocol, m_connectInfo.credentials,
                                          std::chrono::milliseconds(m_connectInfo.timeout),
                                          std::chrono::milliseconds(m_connectInfo.retryInterval),
                                          std::chrono::milliseconds(m_connectInfo.webSocketPingInterval)))
            {
                e_log("websocektclietn connect failed");
                return false;
            }
            return true;
        }

        bool OCPPClient::doSend(const std::string payload)
        {
            d_log("send payload:%s", payload.c_str());
            if (false == m_webSocketClient.send(payload.c_str(), payload.size()))
            {
                e_log("wesockclient send error");
                return false;
            }
            return true;
        }

        void OCPPClient::sendThreadFunc()
        {
            pthread_setname_np(pthread_self(), "CLNT_sendThread");
            while (m_isRunning)
            {
                std::unique_lock<std::mutex> lock(m_sendMutex);
                m_sendCV.wait(lock, [this]
                              { return !m_sendQueue.empty() || !m_isRunning; });

                while (false == m_sendQueue.empty())
                {
                    std::string message = std::move(m_sendQueue.front());
                    m_sendQueue.pop();
                    lock.unlock();
                    if (!doSend(message))
                    {
                        e_log("Failed to send message.");
                    }
                    lock.lock();
                }
            }
        }

        void OCPPClient::parseMessage(const std::string& msg)
        {
            d_log("Received message: %s", msg.c_str());
            MessageType msgType;

            rapidjson::Document doc;
            if (false == validateOCPPMessage(msg, msgType, doc))
            {
                e_log("Invalid OCPP message format, error msg;%s", msg);
                return;
            }

            std::string uniqueId(doc[1].GetString());
            switch (msgType)
            {
            case MessageType::CallResult:
            {
                std::string action("");
                bool hasPendingRequest = false;
                {
                    std::unique_lock<std::mutex> lock(m_pendingRequestsMutex);
                    auto itMsg = m_pendingRequests.find(uniqueId);
                    auto itTime = m_sendTimeMap.find(uniqueId);
                    if (itMsg != m_pendingRequests.end() && itTime != m_sendTimeMap.end()) /* 消息存在 */
                    {
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - itTime->second).count(); /* 运行时间 */
                        if (elapsed > 2500)
                        {
                            e_log("CallResult timeout[2500ms] for uniqueId: %s", uniqueId.c_str());
                            m_sendTimeMap.erase(itTime);
                            return;
                        }
                        action = itMsg->second;
                        hasPendingRequest = true;
                        m_pendingRequests.erase(itMsg);
                        m_sendTimeMap.erase(itTime);
                    }
                }
                if (true == hasPendingRequest)
                {
                    std::unique_lock<std::mutex> lock(m_handlerMutex);
                    auto handlerIt = m_callResultHandlers.find(action);
                    if (handlerIt != m_callResultHandlers.end())
                    {
                        handlerIt->second(doc); /* 处理任务 */
                    }
                    else
                    {
                        w_log("No handler for CallResult: %s", action.c_str());
                    }
                }
                else
                {
                    d_log("No pending request for uniqueId: %s", uniqueId.c_str());
                }
                break;
            }
            case MessageType::CallError:
            {
                std::string action("");
                bool hasPendingRequest = false;
                {
                    std::unique_lock<std::mutex> lock(m_pendingRequestsMutex);
                    auto itMsg = m_pendingRequests.find(uniqueId);
                    auto itTime = m_sendTimeMap.find(uniqueId);
                    if (itMsg != m_pendingRequests.end() && itTime != m_sendTimeMap.end())
                    {
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - itTime->second).count();
                        // 2500 ms 超时 timeout
                        if (elapsed > 2500)
                        {
                            e_log("CallError timeout[2500ms] for uniqueId: %s", uniqueId.c_str());
                            m_sendTimeMap.erase(itTime);
                            return;
                        }
                        action = itMsg->second;
                        hasPendingRequest = true;
                        m_pendingRequests.erase(itMsg);
                        m_sendTimeMap.erase(itTime);
                    }
                }
                if (true == hasPendingRequest)
                {
                    std::unique_lock<std::mutex> lock(m_handlerMutex);
                    auto handlerIt = m_callErrorHandlers.find(action);
                    if (handlerIt != m_callErrorHandlers.end())
                    {
                        handlerIt->second(doc);
                    }
                    else
                    {
                        e_log("No handler for CallError: %s", action.c_str());
                    }
                }
                else
                {
                    d_log("No pending request for uniqueId: %s", uniqueId.c_str());
                }
                break;
            }
            case MessageType::Call:
            {
                std::string action = doc[2].GetString();
                {
                    std::unique_lock<std::mutex> lock(m_handlerMutex);
                    auto handlerIt = m_callHandlers.find(action);
                    if (handlerIt != m_callHandlers.end())
                    {
                        handlerIt->second(doc);
                    }
                    else
                    {
                        e_log("No handler for Call: %s", action.c_str());
                    }
                }
                break;
            }
            default:
                e_log("Unknown message type:%u", msgType);
                break;
            }

        }

        void OCPPClient::recvThreadFunc()
        {
            pthread_setname_np(pthread_self(), "recvThread");
            while (m_isRunning)
            {
                std::unique_lock<std::mutex> lock(m_recvMutex);
                m_recvCV.wait(lock, [this]()
                              { return !m_recvQueue.empty() || !m_isRunning; });

                while (!m_recvQueue.empty())
                {
                    std::string message = std::move(m_recvQueue.front());
                    m_recvQueue.pop();
                    lock.unlock();

                    m_threadPool.addTask("recvThread-parseMessage", [this, message]()
                                         { parseMessage(message); });
                    lock.lock();
                }
            }
        }
    }
}
