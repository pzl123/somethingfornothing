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
            std::string m_cfgPath;
            // 事件处理器引用，处理 ChargePoint 的事件
            IChargePointHandler&        m_eventsHandler; // ChargePointHandler
            std::shared_ptr<ITimerPool> m_timerPoolPtr;  // TimerPool
            std::shared_ptr<ThreadPool> m_threadPoolPtr; // threadPool

            std::unique_ptr<client::WebSocketClient> m_wsClientPtr; // WebSocketClient
            std::unique_ptr<client::IOCPPClient> m_ocppClientPtr;   // OCPPClient
            std::unique_ptr<JsonValidator> m_jsonValidatorPtr;      // JsonValidator
            std::unique_ptr<Connectors> m_connectorsPtr;            // Connectors
            std::unique_ptr<txn::ITXDBHandle> m_txDBHandlePtr;
            Database m_database; // Database
        };
    } // namespace chargepoint
} // namespace ocpp1_6
#endif /* CHARGEPOINT_H */
