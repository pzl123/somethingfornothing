#include <memory>

#include "ChargePoint.h"

#include "ocpp/log/ocpp_log.h"

#include "ocpp/action/Action.h"
#include "ocpp/action/transaction/ITXDBHandle.h"
#include "ocpp/config/ConfigManager.h"
#include "ocpp/version/Version.h"

/*
*  +------------------+       setOCPPClientListener(this)
* |   ChargePoint    | ------------------------------------+
* | (implements      |                                     |
* |  IOCPPClientListener)                                  |
* +------------------+                                     |
*         ↑                                                |
*         | implements                                     |
*         |                                                ↓
* +------------------+        owns          +----------------------------+
* | OCPPClient       | <------------------- | WebSocketClient, etc.      |
* |                  |                      +----------------------------+
* +------------------+
*         ↑
*         | calls back via virtual functions
*         |
* +------------------+
* | Central System   |
* | (CSMS)           |
* +------------------+
*
*/

#define DEFAULT_CONFIG_PATH "/home/zlgmcu/project/learnC++/config_profile/app/ocpp/config/ocpp_default_config.json"

namespace ocpp1_6
{
    namespace chargepoint
    {
        bool ChargePoint::initComponents()
        {
            if (!m_ocppClientPtr)
            {
                log_error("m_ocppClientPtr is not initialized");
                return false;
            }
            if (!m_timerPoolPtr)
            {
                log_error("m_timerPoolPtr is not initialized");
                return false;
            }
            if (!m_threadPoolPtr)
            {
                log_error("m_threadPoolPtr is not initialized");
                return false;
            }

            m_statusNotificationPtr = std::make_unique<state::StatusNotification>(*m_ocppClientPtr);
            if (!m_statusNotificationPtr)
            {
                log_error("StatusNotification initialization failed");
                return false;
            }

            unsigned int connectorNum = m_eventsHandler.getConnectorNum();
            m_connectorsPtr = std::make_unique<Connectors>(m_database, *m_timerPoolPtr, *m_threadPoolPtr, connectorNum, *m_statusNotificationPtr);
            if (!m_connectorsPtr || !m_connectorsPtr->initDatabaseTable())
            {
                log_error("Connectors initialization failed");
                return false;
            }

            // m_txDBHandlePtr = std::make_unique<ocpp1_6::txn::TxDBHandle>(m_database);
            // if (!m_txDBHandlePtr || !m_txDBHandlePtr->initDatabaseTable())
            // {
            //     log_error("TxDBHandle initialization failed");
            //     return false;
            // }

            // m_authentCachePtr = std::make_unique<auth::AuthentCache>(m_database);
            // if (!m_authentCachePtr || !m_authentCachePtr->initDatabaseTable())
            // {
            //     log_error("AuthentCache initialization failed");
            //     return false;
            // }
        }



        bool ChargePoint::initNetWorkModel()
        {
            m_wsClientPtr = std::make_unique<ocpp1_6::client::WebSocketClient>();
            if (m_wsClientPtr == nullptr)
            {
                e_log("WebSocketClient is not initialized.");
                return false;
            }
            if (m_threadPoolPtr == nullptr)
            {
                e_log("ThreadPool is not initialized.");
                return false;
            }

            if (m_jsonValidatorPtr == nullptr)
            {
                e_log("JsonValidator is not initialized.");
                return false;
            }

            uint32_t connectorNum = m_eventsHandler.getConnectorNum();
            m_ocppClientPtr       = std::make_unique<ocpp1_6::client::OCPPClient>(*m_wsClientPtr, *m_jsonValidatorPtr, connectorNum);
            if (m_ocppClientPtr == nullptr)
            {
                log_info("OCPPClient is not initialized.");
                return false;
            }

            /* 这里的 this 是 ChargePoint* 类型。
               但由于 ChargePoint 继承自 IOCPPClient::IOCPPClientListener，
               C++ 编译器会自动将 this 转换为 IOCPPClient::IOCPPClientListener* 类型（隐式向上转型）
            */
            m_ocppClientPtr->setOCPPClientListener(this);
            return m_ocppClientPtr == nullptr ? false : true;
        }

        bool ChargePoint::initConfigModel()
        {
            config::ConfigManager &configManager = config::ConfigManager::getInstance();
            configManager.initConfig(DEFAULT_CONFIG_PATH, m_cfgPath);

            rapidjson::Document deviceConfig;
            uint32_t connectorNum = m_eventsHandler.getConnectorNum();
            std::string ver = getOcppSoftwareVersion();

            if (!configManager.getConfig("", deviceConfig) || !deviceConfig.IsObject() || !deviceConfig.HasMember("ChargePoint") ||
                !deviceConfig["ChargePoint"].IsObject() || !deviceConfig["ChargePoint"].HasMember("DatabasePath") ||
                !deviceConfig["ChargePoint"]["DatabasePath"].IsString() || !deviceConfig["ChargePoint"].HasMember("JsonSchemasPath") ||
                !deviceConfig["ChargePoint"]["JsonSchemasPath"].IsString() || !deviceConfig["ChargePoint"].HasMember("OcppSoftwareVersion") ||
                !deviceConfig["ChargePoint"]["OcppSoftwareVersion"].IsString() || !deviceConfig.HasMember("OCPP1_6") ||
                !deviceConfig["OCPP1_6"].IsObject() || !deviceConfig["OCPP1_6"].HasMember("NumberOfConnectors") ||
                !deviceConfig["OCPP1_6"]["NumberOfConnectors"].IsUint())
            {
                e_log("get config failed.");
                return false;
            }
            auto &allocator = deviceConfig.GetAllocator();

            if (deviceConfig.HasMember("OCPP1_6"))
            deviceConfig["OCPP1_6"]["NumberOfConnectors"] = connectorNum;
            deviceConfig["ChargePoint"]["OcppSoftwareVersion"].SetString(ver.c_str(), allocator);

            if (!config::ConfigManager::getInstance().setConfig("", deviceConfig))
            {
                e_log("save ChargePoint config failed");
            }

            // 加载数据库
            std::string dbPath = deviceConfig["ChargePoint"]["DatabasePath"].GetString();
            if (!m_database.open(dbPath))
            {
                e_log("inti database model");
                return false;
            }

            std::string schemaPath = deviceConfig["ChargePoint"]["JsonSchemasPath"].GetString();
            m_jsonValidatorPtr = std::make_unique<JsonValidator>(schemaPath);
            if (!m_jsonValidatorPtr)
            {
                e_log("Failed to create JsonValidator.");
                return false;
            }
            return true;
        }

    } // namespace chargepoint
} // namespace ocpp1_6
