#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

/**
 * @file ConfigManager.h
 * @brief 配置管理器类定义
 *
 * 提供配置文件的加载、读取、修改、保存以及订阅通知功能。
 * 支持多级配置项访问（如 "database.host"）和线程安全操作。
 */

#include "ocpp/config/ConfigDefaults.h"
#include "ocpp_json_minimal.h"
#include "ocpp/tool/threadpool/ThreadPool.h"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ocpp1_6
{
    namespace config
    {
        /**
         * @class ConfigManager
         * @brief 全局唯一的配置管理器单例类
         *
         * 负责管理应用程序的所有配置信息，包括从文件加载、运行时查询与更新、
         * 自动持久化以及变更事件的通知机制。
         */
        class ConfigManager
        {
        public:
            /**
             * @brief 禁止拷贝构造函数
             */
            ConfigManager(const ConfigManager &) = delete;
            /**
             * @brief 禁止拷贝赋值运算符
             */
            ConfigManager &operator=(const ConfigManager &) = delete;
            /**
             * @brief 禁止移动构造函数
             */
            ConfigManager(ConfigManager &&) = delete;
            /**
             * @brief 禁止移动赋值运算符
             */
            ConfigManager &operator=(ConfigManager &&) = delete;

            /**
             * @brief 回调函数类型定义
             *
             * 当被监听的配置项发生变化时，将触发注册的回调函数，
             * 并传递新的配置值作为参数。
             * @note 不要在回调中执行耗时操作
             */
            using Callback = std::function<void(const rapidjson::Value &)>;

            /**
             * @brief 获取配置管理器实例（线程安全）
             *
             * 使用 C++11 的局部静态变量特性实现线程安全的单例模式。
             *
             * @return ConfigManager& 返回全局唯一实例的引用
             */
            static ConfigManager &getInstance();


            /**
             * @brief 初始化配置管理器
             *
             * 按照优先级顺序尝试加载配置：
             * 1. 缓存配置文件 (cachedCfgPath)
             * 2. 默认配置文件 (defaultCfgPath)
             * 3. 内存中的默认配置
             *
             * @param defaultCfgPath 默认配置文件路径
             * @param cachedCfgPath 缓存配置文件路径（具有最高优先级）
             */
            void initConfig(const std::string &defaultCfgPath, const std::string &cachedCfgPath);

            /**
             * @brief 查询指定键的配置值
             *
             * 支持使用点号分隔的层次结构路径（例如："server.port"）。
             * 如果 key 为空字符串，则表示获取整个配置文档。
             *
             * @param key 配置项的键名或路径
             * @param out 输出参数，用于接收查找到的配置值副本
             * @return bool 成功找到对应配置则返回 true，否则返回 false
             */
            bool getConfig(const std::string &key, rapidjson::Document &out) const;

            /**
             * @brief 更新指定键的配置值
             *
             * 支持使用点号分隔的层次结构路径（例如："server.port"）。
             * 如果 key 为空字符串，则替换整个配置文档。
             * 修改后会在后台延时保存至缓存文件。
             *
             * @param key 配置项的键名或路径
             * @param value 新的配置值（会被深拷贝）
             * @return bool 成功更新配置则返回 true，否则返回 false
             */
            bool setConfig(const std::string &key, const rapidjson::Value &value);

            /**
             * @brief 恢复默认配置
             *
             * 删除现有的缓存配置文件，并重新初始化配置数据。
             *
             * @return bool 成功恢复则返回 true，否则返回 false
             */
            bool restoreConfig();

            /**
             * @brief 注册配置更改监听器
             *
             * 对于给定的配置键，当其值发生改变时，会触发相应的回调函数。
             * 支持通配符路径匹配，如订阅 "server" 将收到所有 server.* 子项的变化通知。
             *
             * @param key 监听的目标配置项键名或路径,key 不能为empty
             * @param cb 配置变化时要调用的回调函数
             * @return uint32_t 分配的订阅 ID，可用于后续取消订阅
             */
            uint32_t subscribe(const std::string &key, Callback cb);

            /**
             * @brief 取消已注册的配置更改监听器
             *
             * 根据订阅 ID 移除之前添加的监听器。
             *
             * @param id 要移除的订阅 ID
             */
            void unsubscribe(uint32_t id);

            /**
             * @brief 保存缓存配置
             *
             * 将当前配置保存到缓存文件中
             */
            void saveCachedConfig();

        private:
            /**
             * @brief 构造函数私有化以实现单例模式
             *
             * 初始化一个拥有两个工作线程的线程池。
             */
            ConfigManager();

            /**
             * @brief 析构函数
             */
            ~ConfigManager();

            /**
             * @brief 从指定路径加载 JSON 文件内容到 RapidJSON 文档对象中
             *
             * @param path JSON 配置文件路径
             * @param doc 用于存储解析结果的 RapidJSON 文档对象
             * @return bool 成功加载并解析则返回 true，否则返回 false
             */
            bool loadJsonFromFile(const std::string &path, rapidjson::Document &doc);

            /**
             * @brief 将 RapidJSON 文档对象保存为格式化的 JSON 文件
             *
             * @param doc 包含待保存配置数据的 RapidJSON 文档对象
             * @param path 目标文件路径
             * @return bool 成功写入文件则返回 true，否则返回 false
             */
            bool saveJsonToFile(const rapidjson::Document &doc, const std::string &path);

            /**
             * @brief 查找指定路径对应的配置节点
             *
             * 主要用于 getConfig 方法中定位目标配置项。
             *
             * @param key 配置项路径
             * @param node 输出参数，指向查找到的节点指针
             * @return bool 找到对应节点则返回 true，否则返回 false
             */
            bool findNode(const std::string &key, const rapidjson::Value *&node) const;

            /**
             * @brief 查找或创建指定路径的父节点
             *
             * 主要用于 setConfig 方法，在设置新值前确保路径上的所有中间节点都存在。
             *
             * @param key 配置项路径
             * @param parent 输出参数，指向最后一个存在的父节点
             * @param lastKey 输出参数，最后一个路径段名称
             * @return bool 成功找到或创建了路径则返回 true，否则返回 false
             */
            bool findOrCreateParentNode(const std::string &key, rapidjson::Value *&parent, std::string &lastKey);

            /**
             * @brief 触发符合条件的监听器回调
             *
             * 当某个配置项发生变更时，遍历所有订阅者并调用匹配的回调函数。
             *
             * @param key 发生变化的配置项路径
             * @param newValue 新的配置值
             */
            void triggerCallbacks(const std::string &modifyPath);

            std::vector<std::string> diffJsonObjects(const std::string &parentPath, const rapidjson::Value &oldObj, const rapidjson::Value &newObj);

        private:
            rapidjson::Document m_currentDoc; ///< 当前有效的配置文档（受 m_cfgMutex 保护）
            std::string m_defaultCfgPath;     ///< 默认配置文件路径
            std::string m_cachedCfgPath;      ///< 缓存配置文件路径

            /// 订阅关系表：<配置路径, <订阅ID, 回调函数>>
            std::unordered_map<std::string, std::unordered_map<uint32_t, Callback>> m_subscriptions;
            std::atomic<uint32_t> m_nextSubId{1}; ///< 订阅ID生成器（0为无效ID）

            std::atomic<bool> m_isSavePending{false};  ///< 控制是否已有待处理的保存任务
            mutable std::mutex m_cfgMutex;             ///< 保护配置文档访问的互斥锁
            std::mutex m_subMutex;                     ///< 保护订阅关系表访问的互斥锁
            ThreadPool m_pool;                         ///< 异步任务执行线程池
        };

    } // namespace config
} // namespace ocpp1_6

#endif