#ifndef CHARGEPOINT_H
#define CHARGEPOINT_H

#include <memory>

#include "ocpp/interface/IChargePoint.h"
#include "ocpp/interface/IChargePointHandler.h"
#include "ocpp/client/OCPPClient.h"

#include "ocpp/tool/timer/TimerPool.h"
#include "ocpp/tool/threadpool/ThreadPool.h"
#include "ocpp/Connector/Connectors.h"
#include "ocpp/action/transaction/ITXDBHandle.h"

namespace ocpp1_6
{
    namespace chargepoint
    {
        class ChargePoint : public IChargePoint, public client::IOCPPClient::IOCPPClientListener
        {
        public:
        private:
            bool initConfigModel();

            bool initNetWorkModel();

            bool initComponents();

            std::string m_cfgPath;
            // 事件处理器引用，处理 ChargePoint 的事件
            IChargePointHandler&        m_eventsHandler; // ChargePointHandler
            std::shared_ptr<ITimerPool> m_timerPoolPtr;  // TimerPool
            std::shared_ptr<ThreadPool> m_threadPoolPtr; // threadPool

            std::unique_ptr<client::WebSocketClient> m_wsClientPtr; // WebSocketClient 连接平台
            std::unique_ptr<client::IOCPPClient> m_ocppClientPtr;   // OCPPClient 收到WebsocketClient信息，处理并通知chargepoint。收到chargepoint信息，并发送给websocketclient
            std::unique_ptr<JsonValidator> m_jsonValidatorPtr;      // JsonValidator
            std::unique_ptr<Connectors> m_connectorsPtr;            // Connectors 枪容器 存储每把枪信息
            std::unique_ptr<txn::ITXDBHandle> m_txDBHandlePtr;
            Database m_database; // Database

            /* 各个 Action 模块， start 方法中进行初始化 */
            std::unique_ptr<state::StatusNotification> m_statusNotificationPtr;

            // 鉴权
            // std::unique_ptr<auth::Authorize> m_authorizePtr;
            // std::unique_ptr<auth::AuthorizeLocalList> m_authorizeLocalListPtr;
            // std::unique_ptr<auth::AuthentCache> m_authentCachePtr;
            // std::unique_ptr<auth::ClearCache> m_clearCachePtr;
        };

    } // namespace chargepoint
} // namespace ocpp1_6
#endif /* CHARGEPOINT_H */
