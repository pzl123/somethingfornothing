#include "OCPPClient.h"
#include "ocpp/common/Common.h"
#include "utils/utils.h"

#include "ocpp/config/chargePointConfigurationKeynames.h"
#include "ocpp/config/standardConfigurationKeyNames.h"

namespace ocpp1_6
{
    namespace client
    {

        bool OCPPClient::initConnectInfo()
        {
            rapidjson::Document jConfig;
            jConfig.SetObject();

            if (!config::ConfigManager::getInstance().getConfig("", jConfig))
            {
                e_log("Get config failed");
                return false;
            }

            // ---------- 基础校验 ----------

            auto cp = jConfig.FindMember("ChargePoint");

            if (cp == jConfig.MemberEnd() || !cp->value.IsObject())
            {
                e_log("config missing or invalid: [ChargePoint]");
                return false;
            }

            auto ocpp = jConfig.FindMember("OCPP1_6");
            if (ocpp == jConfig.MemberEnd() || !ocpp->value.IsObject())
            {
                e_log("config missing or invalid: [OCPP1_6]");
                return false;
            }

            const rapidjson::Value &jChargePoint = cp->value;
            const rapidjson::Value &jOCPP1_6 = ocpp->value;

            // ---------- 详细字段校验 ----------
            auto hasRequired = [](const rapidjson::Value &obj, std::initializer_list<const char *> keys)
            {
                for (auto k : keys)
                {
                    if (!obj.HasMember(k))
                        return false;
                }
                return true;
            };

            if (!hasRequired(jChargePoint,
                             {"Protocol",
                              "ConnectionTimeout",
                              "ConnectionUrl",
                              "ChargePointIdentifier",
                              "SecurityProfile",
                              "RetryInterval",
                              "CallRequestTimeout"}))
            {
                e_log("ChargePoint config missing required fields");
                return false;
            }

            if (!hasRequired(jOCPP1_6, {"AuthorizationKey", "WebSocketPingInterval", "TransactionMessageAttempts"}))
            {
                e_log("OCPP1_6 config missing required fields");
                return false;
            }

            // ---------- 提取必要字段 ----------
            Credentials credentials;

            unsigned int security_profile = jChargePoint["SecurityProfile"].GetUint();
            std::string authorization_key = jOCPP1_6["AuthorizationKey"].GetString();
            std::string url = jChargePoint["ConnectionUrl"].GetString();

            if (!url.empty() && url.back() != '/')
            {
                url += "/";
            }
            url += Url::encode(jChargePoint["ChargePointIdentifier"].GetString());

            // ---------- Basic Auth ----------
            if (!authorization_key.empty() && (security_profile <= 2))
            {
                credentials.user = jChargePoint["ChargePointIdentifier"].GetString();
                credentials.password = authorization_key;
            }

            // ---------- TLS 配置项 ----------
            auto hasTlsConfig = [&](const rapidjson::Value &v)
            {
                return hasRequired(v,
                                   {"Tlsv12CipherList",
                                    "Tlsv13CipherList",
                                    "InternalCertificateManagementEnabled",
                                    "TlsServerCertificateCa",
                                    "TlsClientCertificate",
                                    "TlsClientCertificatePrivateKey",
                                    "TlsClientCertificatePrivateKeyPassphrase",
                                    "TlsAllowSelfSignedCertificates",
                                    "TlsAllowExpiredCertificates",
                                    "TlsAcceptNonTrustedCertificates",
                                    "TlsSkipServerNameCheck"});
            };

            if (security_profile != 1)
            {

                if (!hasTlsConfig(jChargePoint))
                {
                    e_log("Failed to load TLS configuration");
                    return false;
                }

                credentials.tls12_cipher_list = jChargePoint["Tlsv12CipherList"].GetString();
                credentials.tls13_cipher_list = jChargePoint["Tlsv13CipherList"].GetString();

                bool internalCertMgmt = jChargePoint["InternalCertificateManagementEnabled"].GetBool();

                if (security_profile == 0 || internalCertMgmt)
                {
                    // 外部证书
                    credentials.server_certificate_ca = jChargePoint["TlsServerCertificateCa"].GetString();

                    if (security_profile == 0 || security_profile == 3)
                    {
                        // 双向认证
                        credentials.client_certificate = jChargePoint["TlsClientCertificate"].GetString();
                        credentials.client_certificate_private_key = jChargePoint["TlsClientCertificatePrivateKey"].GetString();
                        credentials.client_certificate_private_key_passphrase =
                            jChargePoint["TlsClientCertificatePrivateKeyPassphrase"].GetString();
                    }

                    credentials.allow_selfsigned_certificates = jChargePoint["TlsAllowSelfSignedCertificates"].GetBool();
                    credentials.allow_expired_certificates = jChargePoint["TlsAllowExpiredCertificates"].GetBool();
                    credentials.accept_untrusted_certificates = jChargePoint["TlsAcceptNonTrustedCertificates"].GetBool();
                    credentials.skip_server_name_check = jChargePoint["TlsSkipServerNameCheck"].GetBool();
                    credentials.encoded_pem_certificates = false;
                }
                else
                {
                    // 内部管理证书
                    credentials.server_certificate_ca = "";
                    if (security_profile == 3)
                    {
                        credentials.client_certificate = "";
                        credentials.client_certificate_private_key = "";
                        credentials.client_certificate_private_key_passphrase =
                            jChargePoint["TlsClientCertificatePrivateKeyPassphrase"].GetString();
                    }

                    credentials.encoded_pem_certificates = true;
                    credentials.allow_selfsigned_certificates = true;
                    credentials.allow_expired_certificates = true;
                    credentials.accept_untrusted_certificates = false;
                    credentials.skip_server_name_check = false;
                }
            }

            // ---------- 保存结果 ----------
            m_connectInfo.credentials = credentials;
            m_connectInfo.url = url;
            m_connectInfo.protocol = jChargePoint["Protocol"].GetString();
            m_connectInfo.timeout = jChargePoint["ConnectionTimeout"].GetUint();
            m_connectInfo.messageTimeout = jChargePoint["CallRequestTimeout"].GetUint();
            m_connectInfo.retryInterval = jChargePoint["RetryInterval"].GetInt();
            m_connectInfo.webSocketPingInterval = jOCPP1_6["WebSocketPingInterval"].GetInt();
            m_connectInfo.transactionMessageAttempts = jOCPP1_6["TransactionMessageAttempts"].GetUint();

            i_log("m_connectInfo: %s, protocol:%s, timeout:%u, messageTimeout:%u retryInterval:%d, webSocketPingInterval:%d",
                     m_connectInfo.url.c_str(),
                     m_connectInfo.protocol.c_str(),
                     m_connectInfo.timeout,
                     m_connectInfo.messageTimeout,
                     m_connectInfo.retryInterval,
                     m_connectInfo.webSocketPingInterval);

            return true;
        }

        OCPPClient::OCPPClient(WebSocketClient &ws, JsonValidator &validator, unsigned int connectorNum)
            : m_ws(ws), m_validator(validator), m_connectorNum(connectorNum + 1), m_threadPool(4)
        {
            m_ws.registerListener(this);
        }

        OCPPClient::~OCPPClient()
        {
            stop();
        }

        bool OCPPClient::start()
        {
            if (!initConnectInfo())
                return false;
            if (!doConnect())
                return false;
            if (m_running)
                return false;

            m_running = true;
            // connectors: 0~N
            for (unsigned int i = 0; i <= m_connectorNum; ++i)
            {
                m_connectors.push_back(std::make_unique<ConnectorContext>());
            }

            for (unsigned int i = 0; i <= m_connectorNum; ++i)
            {
                m_sendThreads.emplace_back(&OCPPClient::sendThread, this, i);
            }

            m_retryThread = std::thread(&OCPPClient::retryTimeoutThread, this);

            return true;
        }

        bool OCPPClient::stop()
        {
            m_running = false;

            for (auto &conn : m_connectors)
            {
                conn->running = false;
                conn->cv.notify_all();
            }

            if (m_retryThread.joinable())
                m_retryThread.join();

            {
                std::lock_guard<std::mutex> lk(m_pendingMutex);
                m_pending.clear();
            }

            for (auto &t : m_sendThreads)
            {
                if (t.joinable())
                {
                    t.join();
                }
            }

            if (m_isConnected)
            {
                m_ws.disConnect(true);
            }

            return true;
        }

        bool OCPPClient::disConnect()
        {
            return m_ws.disConnect(true);
        }

        bool OCPPClient::doConnect()
        {
            if (m_isConnected)
            {
                i_log("already connected");
                return true;
            }

            if (m_ws.Connect(m_connectInfo.url,
                             m_connectInfo.protocol,
                             m_connectInfo.credentials,
                             std::chrono::milliseconds(m_connectInfo.timeout),
                             std::chrono::milliseconds(m_connectInfo.retryInterval),
                             std::chrono::milliseconds(m_connectInfo.webSocketPingInterval)))
            {
                return true;
            }
            e_log("connect failed");
            return false;
        }


        bool OCPPClient::isConnected() const
        {
            return m_isConnected;
        }

        bool OCPPClient::sendCall(const std::string &action,
                                  const rapidjson::Document &reqPayload,
                                  rapidjson::Document &rspPayload,
                                  unsigned int connId,
                                  bool isTx,
                                  int timeout)
        {
            if (!isConnected())
            {
                e_log("sendCall: not connected");
                return false;
            }

            if (!m_validator.validateRequest(action, reqPayload))
            {
                e_log("schema validate failed");
                return false;
            }

            if (connId >= m_connectors.size())
            {
                e_log("Invalid connId");
                return false;
            }

            std::string uniqueId = generateMessageId();

            /* [<MessageTypeId>, "<UniqueId>", "<Action>", {Payload}] */
            rapidjson::Document ocppMsg;
            ocppMsg.SetArray();
            auto &alloc = ocppMsg.GetAllocator();
            ocppMsg.PushBack(static_cast<int>(MessageType::Call), alloc);
            ocppMsg.PushBack(rapidjson::Value(uniqueId.c_str(), alloc), alloc);
            ocppMsg.PushBack(rapidjson::Value(action.c_str(), alloc), alloc);
            rapidjson::Value payloadCopy;
            payloadCopy.CopyFrom(reqPayload, alloc);
            ocppMsg.PushBack(payloadCopy, alloc);

            // 序列化
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            ocppMsg.Accept(writer);
            std::string raw = buffer.GetString();

            auto pc = std::make_shared<PendingCall>();
            pc->uniqueId = uniqueId;
            pc->raw = raw;
            pc->isTx = isTx;
            pc->expireAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
            pc->retryIntervalMs = m_connectInfo.retryInterval;
            pc->maxRetry = m_connectInfo.transactionMessageAttempts;
            pc->nextRetry = std::chrono::steady_clock::now() + std::chrono::milliseconds(pc->retryIntervalMs);
            pc->connId = connId;

            {
                std::lock_guard<std::mutex> lk(m_pendingMutex);
                m_pending[uniqueId] = pc;
            }

            auto &ctx = *m_connectors[connId];
            isTx ? ctx.txQueue.push(raw) : ctx.nonTxQueue.push(raw); /* 将任务入到对应的connet任务队列中 */
            ctx.cv.notify_one();

            std::unique_lock<std::mutex> lk(pc->mtx);
            if (!pc->cv.wait_until(lk, pc->expireAt, [&]{ return pc->finished; })) /* 同步阻塞，等待cs回应或超时 */
            {
                return false; /* 超时 */
            }

            if (!pc->success)
            {
                return false;
            }
            rspPayload.Parse(pc->response.c_str());
            return true;
        }

        bool OCPPClient::sendCallError(const std::string &uniqueId,
                                       const std::string &errorCode,
                                       const std::string &errorDescription,
                                       const rapidjson::Value &errorDetails,
                                       unsigned int connId,
                                       bool isTx,
                                       int timeout)
        {
            (void)isTx;
            (void)timeout;

            if (!isConnected())
            {
                e_log("sendCallError: not connected");
                return false;
            }

            /* [<MessageTypeId>, "<UniqueId>",  "<errorCode>",  "<errorDescription>",  {<errorDetails>}] */
            rapidjson::Document doc;
            doc.SetArray();
            auto &a = doc.GetAllocator();
            doc.PushBack(static_cast<int>(MessageType::CallError), a);
            doc.PushBack(rapidjson::Value(uniqueId.c_str(), a), a);
            doc.PushBack(rapidjson::Value(errorCode.c_str(), a), a);
            doc.PushBack(rapidjson::Value(errorDescription.c_str(), a), a);
            rapidjson::Value cp;
            cp.CopyFrom(errorDetails, a);
            doc.PushBack(cp, a);

            rapidjson::StringBuffer b;
            rapidjson::Writer<rapidjson::StringBuffer> w(b);
            doc.Accept(w);

            auto &c = *m_connectors[connId];
            c.nonTxQueue.push(b.GetString());
            c.cv.notify_one();
            return true;
        }

        bool OCPPClient::sendCallResult(const std::string &uniqueId,
                                        const std::string &action,
                                        const rapidjson::Document &payload,
                                        unsigned int connId,
                                        bool isTx, int timeout)
        {
            (void)isTx;
            (void)timeout;

            if (!isConnected())
            {
                e_log("sendCallResult: not connected");
                return false;
            }

            if (!m_validator.validateResponse(action, payload))
            {
                e_log("schema validate failed");
                return false;
            }

            /* [<MessageTypeId>, "<UniqueId>", {Payload}] */
            rapidjson::Document doc;
            doc.SetArray();
            auto &a = doc.GetAllocator();
            rapidjson::Value cp;
            cp.CopyFrom(payload, a);
            doc.PushBack(static_cast<int>(MessageType::CallResult), a);
            doc.PushBack(rapidjson::Value(uniqueId.c_str(), a), a);
            doc.PushBack(cp, a);

            rapidjson::StringBuffer b;
            rapidjson::Writer<rapidjson::StringBuffer> w(b);
            doc.Accept(w);

            auto &c = *m_connectors[connId];
            c.nonTxQueue.push(b.GetString());
            c.cv.notify_one();
            return true;
        }

        void OCPPClient::registerCallHandler(const std::string &action, std::function<void(rapidjson::Document &)> handler)
        {
            d_log("registerCallHandler: %s", action.c_str());
            std::unique_lock<std::mutex> lock(m_callHandlerMtx);
            m_callHandlers[action] = handler;
        }

        void OCPPClient::setOCPPClientListener(IOCPPClientListener *listener)
        {
            if (listener != nullptr && m_listener == nullptr)
            {
                i_log("OCPPClientListener has been set.");
                m_listener = listener;
                /* 由于 m_listener 指向的是 ChargePoint 对象（虽然是 IOCPPClientListener* 类型），
                   虚函数机制会动态绑定到 ChargePoint 的实现。 */
            }
        }

        void OCPPClient::wsClientConnected()
        {
            i_log("websocket is connected");
            m_isConnected = true;
            if (m_listener)
            {
                m_listener->onConnected(); /* 通知到chargepoint */
            }
        }

        void OCPPClient::wsClientFailed()
        {
            i_log("websocket is connect failed");
            m_isConnected = false;
            if (m_listener)
            {
                m_listener->onFailed();
            }
        }

        void OCPPClient::wsClientDisconnected()
        {
            i_log("websocket disconnected");
            m_isConnected = false;
            if (m_listener)
            {
                m_listener->onDisconnected();
            }
        }

        void OCPPClient::wsClientError()
        {
            i_log("websocket is connect error");
            m_isConnected = false;
            if (m_listener)
            {
                m_listener->onError();
            }
        }

        void OCPPClient::wsClientDataReceived(const void *data, size_t size)
        {
            std::string message(static_cast<const char *>(data), size);
            parseMessage(message);
        }

        void OCPPClient::parseMessage(const std::string &message)
        {
            rapidjson::Document doc;
            MessageType type;

            // 消息格式验证
            if (!validateOCPPMessage(message, type, doc))
                return;

            // 提取 uniqueId
            const char *uniqueId = doc[1].GetString();

            if (type == MessageType::CallResult || type == MessageType::CallError) /* 如果是回复的Call */
            {
                std::shared_ptr<PendingCall> pc;

                {
                    std::lock_guard<std::mutex> lk(m_pendingMutex);
                    auto it = m_pending.find(uniqueId);
                    if (it == m_pending.end())
                    {
                        e_log("uniqueId:%s error in m_pengind, message:%s", uniqueId, message.c_str());
                        return;
                    }
                    pc = it->second;
                }

                {
                    std::lock_guard<std::mutex> lk(pc->mtx);
                    if (true == pc->finished)
                    {
                        e_log("uniqueId:%s error in m_pengind, message:%s", uniqueId, message.c_str());
                        return;
                    }
                    pc->finished = true;
                    pc->success = (type == MessageType::CallResult) ? true : false;
                    pc->response = message;
                }
                pc->cv.notify_all();
            }
            else if (type == MessageType::Call) /* 如果是下发的命令 */
            {
                const char *action = doc[2].GetString();
                std::function<void(rapidjson::Document &)> handler;
                {
                    std::lock_guard<std::mutex> lk(m_callHandlerMtx);
                    auto it = m_callHandlers.find(action);
                    if (it == m_callHandlers.end())
                    {
                        w_log("No handler for action: %s", action);
                        return;
                    }
                    handler = it->second;
                }

                auto msg = std::make_shared<rapidjson::Document>(); /* 由对应的handler释放内存 */
                msg->CopyFrom(doc, msg->GetAllocator());
                rapidjson::Document payloadDoc;
                payloadDoc.CopyFrom(doc[3], payloadDoc.GetAllocator());

                d_log("Dispatching Call: %s (UID=%s)", action, uniqueId);

                if (m_validator.validateRequest(action, payloadDoc))
                {
                    /* 入线程池处理任务 将handler 和 msg */
                    m_threadPool.enqueue([handler, msg]()
                        {
                            try
                            {
                                handler(*msg);
                            }
                            catch (const std::exception &e)
                            {
                                e_log("handler exception: %s", e.what());
                            }
                            catch (...)
                            {
                                e_log("handler unknown exception");
                            }
                        });
                }
            }
            else
            {
                e_log("Unknown message type: %d", (int)type);
            }
        }


        bool OCPPClient::doSend(const std::string& msg)
        {
            // i_log("send:%s", msg.c_str());
            return m_ws.send(msg.c_str(), msg.size());
        }

        void OCPPClient::retryTimeoutThread()
        {
            pthread_setname_np(pthread_self(), "ocpp-retrytimeout");
            while (m_running)
            {
                auto now = std::chrono::steady_clock::now();

                std::vector<std::shared_ptr<PendingCall>> retryList;
                std::vector<std::string> eraseList;

                {
                    std::lock_guard<std::mutex> lock(m_pendingMutex);

                    for (auto &[uid, pendingCall] : m_pending)
                    {
                        if (pendingCall->finished) /* 已收到回复 */
                        {
                            eraseList.push_back(uid); /* 清除 */
                            continue;
                        }

                        /* pendingCall 是交易信息 且 到达超时点 且 重发次数未到达最大次数 */
                        if (pendingCall->isTx && now >= pendingCall->nextRetry && pendingCall->retryCount < pendingCall->maxRetry)
                        {
                            retryList.push_back(pendingCall); /* 重发 */
                        }
                        else if (now >= pendingCall->expireAt) /* 到期 */
                        {
                            std::lock_guard<std::mutex> lk(pendingCall->mtx);
                            if (!pendingCall->finished)
                            {
                                pendingCall->finished = true;
                                pendingCall->success = false;
                                pendingCall->cv.notify_all();
                            }
                            eraseList.push_back(uid); /* 清除 */
                        }
                    }

                    for (auto &uid : eraseList)
                    {
                        m_pending.erase(uid);
                    }
                }

                for (auto &pendingCall : retryList)
                {
                    {
                        std::lock_guard<std::mutex> lk(pendingCall->mtx);
                        if (pendingCall->finished)
                        {
                            continue;
                        }
                        pendingCall->retryCount++;
                    }

                    size_t capped = std::min<std::size_t>(static_cast<std::size_t>(pendingCall->retryCount), static_cast<std::size_t>(8));
                    auto delayMs = pendingCall->retryIntervalMs * (1u << capped);

                    pendingCall->nextRetry = now + std::chrono::milliseconds(delayMs);

                    if (pendingCall->connId < m_connectors.size())
                    {
                        m_connectors[pendingCall->connId]->txQueue.push(pendingCall->raw);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        void OCPPClient::sendThread(unsigned int connectorId)
        {
            pthread_setname_np(pthread_self(), "ocpp-send");

            auto &ctx = *m_connectors[connectorId];

            while (ctx.running)
            {
                std::string msg;

                if (!ctx.txQueue.empty())
                {
                    ctx.txQueue.pop(msg);
                }
                else if (!ctx.nonTxQueue.empty())
                {
                    ctx.nonTxQueue.pop(msg);
                }
                else
                {
                    std::unique_lock<std::mutex> lk(ctx.mtx);
                    ctx.cv.wait_for(lk, std::chrono::milliseconds(100));
                    continue;
                }

                if (msg.empty())
                {
                    std::unique_lock<std::mutex> lk(ctx.mtx);
                    ctx.cv.wait_for(lk, std::chrono::milliseconds(100));
                    continue;
                }
                doSend(msg);
            }
        }

    } // namespace client
} // namespace ocpp1_6
