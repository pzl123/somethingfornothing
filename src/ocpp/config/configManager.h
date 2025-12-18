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
    typedef enum
    {
        ERROR_KEY = 0,
        IMMEDIATE = 1,
        RESETWEBSOCKET = 2,
        RESETOCPP = 3
    } ActionLevel_e;

    class ConfigManager
    {
    public:
        ConfigManager(std::string path);

        ~ConfigManager();
        bool getConfig(const std::string& name, rapidjson::Value& value);
        bool SetConfig(const std::string& name, const rapidjson::Value& value);
        bool loadConfig();
        bool saveCacheConfig();
        uint32_t getChangeConfigActionLevel() { return m_configActionLevel; }
        bool isReadOnly(const std::string& key, bool& bread);
        const rapidjson::Document &getAllConfig() const;
        bool deleteCacheConfigFile(bool deleteCache);


    private:
        void findChangedKeys(const rapidjson::Value &oldVal, const rapidjson::Value &newVal, const std::string &path, std::vector<std::string> &changedKeys);
        bool createDefaultConfig(const std::string &path); /*创建默认配置*/
        ActionLevel_e evaluateConfigActionLevel(const std::vector<std::string>& keys);
        void initReadOnlyConfig(); /* 初始化只读配置 */

        rapidjson::Document m_cacheConfig; /*配置*/
        rapidjson::Document m_cfgOnlyRead; /* 只读配置项 */
        // std::map<std::string, ReadOnlyConfig>   configProperties;    /*存储配置项只读性*/
        std::string m_defaultCfgPath; /*默认配置文件路径*/
        std::string m_currentCfgPath; /*当前配置文件路径*/
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
