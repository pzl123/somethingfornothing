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
#include "ocpp/client/OCPPclient.h"
#include "ocpp/config/configManager.h"
#include "ocpp/interface/IChargePoint.h"
#include "standardConfigurationKeyNames.h"

namespace ocpp1_6
{
    class ConfigManager
    {
    public:
        ConfigManager(std::string path);

        ~ConfigManager();

        /**
         * @brief 获取配置项
         * @param name 配置项名称
         * @param value 配置项值
         * @return true 成功
         */
        bool getConfig(const std::string &name, rapidjson::Document &value);

        /**
         * @brief 设置配置项
         * @param name 配置项名称
         * @param value 配置项值
         */
        bool setConfig(const std::string &name, const rapidjson::Document &value);

        rapidjson::Document getAllConfig();

        bool saveCacheConfig();

        /**
         * @brief 清除缓存配置
         *
         * @param bDeleteCache true:清除缓存，m_current_config指向的配置文件删除
         *                   false:不清除缓存，将m_cache_config存储的配置设置给m_current_config指向的配置文件
         * @return bool
         */
        bool clearCacheConfig(bool bDeleteCache);

        /**
         * @brief 返回配置键是否只读，ture为只读，false为可读写
         *
         * @param key 配置键名称
         * @param bread 是否只读
         * @return true 成功
         * @return false 失败
         */
        bool isReadOnly(const std::string &key, bool &bread);

        /*加载配置*/
        bool loadConfig();

        /**
         * @brief 获取配置修改的级别
         *
         * @return uint32_t 0:不处理 1:直接生效  2: 重连 WebSocket  3：重启 OCPP 模块
         */
        uint32_t getChangeConfigActionLevel() { return m_configActionLevel; }

    private:
        /**
         * @brief 获取配置修改的 key
         *
         * @param oldVal 旧值
         * @param newVal 新值
         * @param path 配置路径
         * @param changedKeys 配置修改的 key
         * @return void
         */
        void findChangedKeys(const rapidjson::Document &oldVal,
                             const rapidjson::Document &newVal,
                             const std::string &path,
                             std::vector<std::string> &changedKeys);

        /**
         * @brief 评估配置修改级别
         *
         * @param keys
         * @return int 0:不处理  1:直接生效  2: 重连 WebSocket  3：重启 OCPP 模块
         */
        uint32_t evaluateConfigActionLevel(const std::vector<std::string> &keys);

        void initReadOnlyConfig(); /*初始化只读配置*/

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
