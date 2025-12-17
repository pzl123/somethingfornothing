#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <optional>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "rapidjson/document.h"

#include "ocpp/credentials/credentials.h"
#include "ocpp/config/configManager.h"
#include "ocpp/interface/IChargePoint.h"

namespace ocpp1_6
{
    class ConfigManager
    {
    public:
        ConfigManager(std::string path);

        ~ConfigManager();

        bool createDefaultConfig(const std::string &path); /*创建默认配置*/

        rapidjson::Document m_cache_config; /*配置*/
        rapidjson::Document m_bCfgRead;
        // std::map<std::string, ReadOnlyConfig>   configProperties;    /*存储配置项只读性*/
        std::string m_default_cfgPath; /*默认配置文件路径*/
        std::string m_current_cfgPath; /*当前配置文件路径*/
        mutable std::mutex m_mutex;    /*互斥锁*/

        uint32_t m_configActionLevel = 0;
        std::unordered_set<std::string> m_restartOcppKeys;
        std::unordered_set<std::string> m_restartWebSocketKeys;
        std::unordered_set<std::string> m_immediateEffectKeys;

        std::shared_ptr<chargepoint::IChargePoint> m_chargePointPtr;
        std::shared_ptr<chargepoint::IChargePointHandler> m_eventsHandler;
    }; // 类 ConfigManager
} // namespace ocpp1_6

#endif // CONFIGMANAGER_H
