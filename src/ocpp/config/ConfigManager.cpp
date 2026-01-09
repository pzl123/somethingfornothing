#include "ConfigManager.h"
#include <sstream>
#include "utils/utils.h"

namespace ocpp1_6
{
    namespace config
    {

        /**
         * @brief 构造函数私有化以实现单例模式
         *
         * 初始化一个拥有两个工作线程的线程池。
         */
        ConfigManager::ConfigManager() : m_pool(2) {}

        ConfigManager::~ConfigManager()
        {
            // saveCachedConfig();
        }

        /**
         * @brief 获取配置管理器实例（线程安全）
         *
         * 使用 C++11 的局部静态变量特性实现线程安全的单例模式。
         *
         * @return ConfigManager& 返回全局唯一实例的引用
         */
        ConfigManager &ConfigManager::getInstance()
        {
            static ConfigManager inst;
            return inst;
        }

        /**
         * @brief 初始化配置管理器
         *
         * 按优先级加载配置：缓存配置 > 默认配置 > 内存默认配置
         * 加载成功后记录当前配置到日志
         */
        void ConfigManager::initConfig(const std::string &defaultCfgPath, const std::string &cachedCfgPath)
        {
            i_log("init ConfigManager, defaultCfgPath=%s, cachedCfgPath=%s", defaultCfgPath.c_str(), cachedCfgPath.c_str());
            std::lock_guard<std::mutex> lock(m_cfgMutex);

            m_defaultCfgPath = defaultCfgPath;
            m_cachedCfgPath = cachedCfgPath + "/ocpp_cache_config.json";

            rapidjson::Document workingDoc;

            // 尝试加载缓存文件
            if (loadJsonFromFile(m_cachedCfgPath, workingDoc))
            {
                i_log("Loaded cached config from: %s", m_cachedCfgPath.c_str());
            }
            // 尝试加载默认文件
            else if (loadJsonFromFile(m_defaultCfgPath, workingDoc))
            {
                i_log("Loaded default config from: %s", defaultCfgPath.c_str());
            }
            // 使用内存默认配置
            else
            {
                workingDoc.Parse(DEFAULT_CONFIG_JSON);
                i_log("Use in-memory default config");
            }

            // 验证并自动补全
            if (!validateAndMergeConfig(workingDoc))
            {
                e_log("Config validation failed, using pure default config instead.");
                workingDoc.Parse(DEFAULT_CONFIG_JSON);
            }

            m_currentDoc.Swap(workingDoc);
            saveJsonToFile(m_currentDoc, m_cachedCfgPath);

            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_currentDoc.Accept(writer);
            i_log("Current config: %s", buffer.GetString());
        }

        /**
         * @brief 获取指定键的配置值
         *
         * 支持层级路径访问（如"a.b.c"），空键表示获取完整配置
         * 返回值通过out参数深拷贝传出
         */
        bool ConfigManager::getConfig(const std::string &key, rapidjson::Document &out) const
        {
            std::lock_guard<std::mutex> lock(m_cfgMutex);

            const rapidjson::Value *node = nullptr;

            // ------ ① 检查 m_currentDoc 是否是对象 ------
            if (!m_currentDoc.IsObject())
            {
                e_log("Config root is not object! type=%d", m_currentDoc.GetType());
                return false;
            }

            // ------ ② 获取节点 ------
            if (key.empty())
            {
                node = &m_currentDoc;
            }
            else
            {
                if (!findNode(key, node))
                {
                    d_log("Config key not found: %s", key.c_str());
                    return false;
                }
            }

            // ------ ③ 初始化 out ------
            out.SetObject();

            // ------ ④ 复制节点（必须用 out 自己的 allocator） ------
            out.CopyFrom(*node, out.GetAllocator());

            return true;
        }

        /**
         * @brief 设置指定键的配置值
         *
         * 支持层级路径设置（如"a.b.c"），空键表示替换完整配置
         * 修改后会触发回调并延迟保存到缓存文件
         */
        bool ConfigManager::setConfig(const std::string &key, const rapidjson::Value &value)
        {
            std::lock_guard<std::mutex> lock(m_cfgMutex);

            // 全配置替换：必须是JSON对象
            if (key.empty())
            {
                if (!value.IsObject())
                {
                    e_log("Set full config failed: value must be JSON object");
                    return false;
                }
                // 对比全配置差异，找出所有被修改的子节点路径
                std::vector<std::string> changedPaths = diffJsonObjects("", m_currentDoc, value);
                // 替换全配置
                m_currentDoc.CopyFrom(value, m_currentDoc.GetAllocator());
                // 仅针对被修改的子节点触发回调
                for (const auto &path : changedPaths)
                {
                    triggerCallbacks(path);
                }
            }
            else
            {
                // 类型匹配（你已实现，保留）
                if (!CompareJsonTypeByPath(key, m_currentDoc, value))
                {
                    e_log("Value type mismatch for key: %s", key.c_str());
                    return false;
                }

                // ===== 核心修改：区分「修改单个子节点」和「替换整个父节点」=====
                const rapidjson::Value *existNode = nullptr;
                bool nodeExist = findNode(key, existNode);

                // 场景1：修改单个子节点（如ChargePoint.ChargePointIdentifier）→ 直接触发
                if (nodeExist && !existNode->IsObject())
                {
                    // 层级配置修改：查找/创建父节点
                    rapidjson::Value *parent = nullptr;
                    std::string lastKey;
                    if (!findOrCreateParentNode(key, parent, lastKey))
                    {
                        e_log("Failed to prepare parent node for key: %s", key.c_str());
                        return false;
                    }

                    // 深拷贝新值到当前文档内存
                    rapidjson::Value newValue;
                    newValue.CopyFrom(value, m_currentDoc.GetAllocator());

                    // 更新值
                    (*parent)[lastKey.c_str()].Swap(newValue);

                    // 触发当前子节点的回调（如ChargePoint.ChargePointIdentifier）
                    triggerCallbacks(key);
                }
                // 场景2：替换整个父节点（如ChargePoint）→ 对比差异，仅触发修改的子节点
                else if (value.IsObject())
                {
                    // 保存旧的父节点值
                    rapidjson::Value oldParentObj;
                    if (nodeExist)
                    {
                        oldParentObj.CopyFrom(*existNode, m_currentDoc.GetAllocator());
                    }
                    // 替换父节点
                    rapidjson::Value *parent = nullptr;
                    std::string lastKey;
                    if (!findOrCreateParentNode(key, parent, lastKey))
                    {
                        e_log("Failed to prepare parent node for key: %s", key.c_str());
                        return false;
                    }
                    rapidjson::Value newValue;
                    newValue.CopyFrom(value, m_currentDoc.GetAllocator());
                    (*parent)[lastKey.c_str()].Swap(newValue);

                    // 对比新旧父节点，找出被修改的子节点路径
                    std::vector<std::string> changedPaths = diffJsonObjects(key, oldParentObj, value);
                    // 仅触发被修改的子节点回调
                    for (const auto &path : changedPaths)
                    {
                        triggerCallbacks(path);
                    }
                }
                else
                {
                    e_log("Set config failed: %s is object, but value is not object", key.c_str());
                    return false;
                }
            }

            // 延迟保存到缓存文件（保留原有逻辑）
            if (!m_cachedCfgPath.empty() && !m_isSavePending.exchange(true))
            {
                i_log("Save cached config to: %s", m_cachedCfgPath.c_str());
                m_pool.enqueue(
                    [this]()
                    {
                        i_log("Save cached config thread started.");
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        std::lock_guard<std::mutex> lock(m_cfgMutex);
                        saveCachedConfig();
                        m_isSavePending = false; // 重置保存标志
                    });
            }

            return true;
        }
        /**
         * @brief 触发配置变化回调
         *
         * 根据修改路径触发所有匹配的订阅回调函数
         */
        void ConfigManager::triggerCallbacks(const std::string &modifyPath)
        {
            std::unique_lock<std::mutex> subLock(m_subMutex);
            std::unordered_map<std::string, std::vector<Callback>> matchedCallbacksMap;

            // 仅匹配「订阅路径 = 修改路径」（精准触发）
            if (auto it = m_subscriptions.find(modifyPath); it != m_subscriptions.end())
            {
                for (const auto &[subId, cb] : it->second)
                {
                    if (cb)
                    {
                        matchedCallbacksMap[modifyPath].emplace_back(cb);
                    }
                }
                // 清理空订阅列表
                if (it->second.empty())
                {
                    m_subscriptions.erase(it);
                }
            }

            subLock.unlock(); // 提前释放订阅表锁

            // 异步触发回调（保留原有逻辑）
            for (const auto &[subscribePath, callbacks] : matchedCallbacksMap)
            {
                for (const auto &cb : callbacks)
                {
                    m_pool.enqueue(
                        [this, subscribePath, cb]() mutable
                        {
                            std::unique_lock<std::mutex> cfgLock(m_cfgMutex);
                            const rapidjson::Value *targetNode = nullptr;
                            if (!findNode(subscribePath, targetNode))
                            {
                                w_log("Callback for path [%s] failed: node not found", subscribePath.c_str());
                                return;
                            }

                            // 深拷贝值（移除const_cast，保留你的类型校验）
                            rapidjson::Value copyVal;
                            copyVal.CopyFrom(*targetNode, m_currentDoc.GetAllocator());
                            cfgLock.unlock();

                            // 执行回调
                            try
                            {
                                cb(copyVal);
                            }
                            catch (const std::exception &e)
                            {
                                e_log("Callback for path [%s] execution failed: %s", subscribePath.c_str(), e.what());
                            }
                            catch (...)
                            {
                                e_log("Unknown error in callback for path [%s]", subscribePath.c_str());
                            }
                        });
                }

                i_log("Triggered %zu callbacks for subscribe path: [%s] (modified path: [%s])",
                         callbacks.size(),
                         subscribePath.c_str(),
                         modifyPath.c_str());
            }

            if (matchedCallbacksMap.empty())
            {
                d_log("No callbacks matched for modified path: [%s]", modifyPath.c_str());
            }
        }
        /**
         * @brief 恢复默认配置
         *
         * 删除缓存文件并重新初始化配置系统
         */
        bool ConfigManager::restoreConfig()
        {
            std::lock_guard<std::mutex> lock(m_cfgMutex);

            // 删除缓存文件（忽略文件不存在的情况）
            if (!m_cachedCfgPath.empty())
            {
                if (remove(m_cachedCfgPath.c_str()) != 0)
                {
                    if (errno != ENOENT)
                    {
                        e_log("Failed to delete cached config [%s]: %s", m_cachedCfgPath.c_str(), strerror(errno));
                        return false;
                    }
                    i_log("Cached config file not exist: %s", m_cachedCfgPath.c_str());
                }
                else
                {
                    i_log("Deleted cached config: %s", m_cachedCfgPath.c_str());
                }
            }

            // 重新初始化配置（此时会加载默认配置）
            initConfig(m_defaultCfgPath, m_cachedCfgPath);
            i_log("Restored default config successfully");
            return true;
        }

        /**
         * @brief 订阅配置变化
         *
         * 注册指定路径配置变化的回调函数，返回订阅ID
         */
        uint32_t ConfigManager::subscribe(const std::string &key, Callback cb)
        {
            if (key.empty())
            {
                e_log("Subscribe failed: key is empty");
                return 0;
            }
            if (!cb)
            {
                e_log("Subscribe failed: callback is null");
                return 0;
            }

            std::lock_guard<std::mutex> lock(m_subMutex);
            const uint32_t subId = m_nextSubId++;
            m_subscriptions[key][subId] = std::move(cb);
            i_log("Subscribe key [%s] with id: %u", key.c_str(), subId);
            return subId;
        }

        /**
         * @brief 取消订阅配置变化
         *
         * 根据订阅ID取消之前注册的配置变化监听
         */
        void ConfigManager::unsubscribe(uint32_t id)
        {
            if (id == 0)
            {
                w_log("Unsubscribe failed: invalid id [0]");
                return;
            }

            std::lock_guard<std::mutex> lock(m_subMutex);
            for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it)
            {
                if (it->second.erase(id) > 0)
                {
                    i_log("Unsubscribe id [%u] from key [%s]", id, it->first.c_str());
                    // 清理空订阅列表
                    if (it->second.empty())
                    {
                        m_subscriptions.erase(it);
                    }
                    return;
                }
            }

            w_log("Unsubscribe failed: id [%u] not found", id);
        }

        void ConfigManager::saveCachedConfig()
        {
            if (m_cachedCfgPath.empty())
            {
                w_log("Cached config path is empty, skip saving");
                return;
            }

            if (!saveJsonToFile(m_currentDoc, m_cachedCfgPath))
            {
                e_log("Failed to save cached config to [%s]", m_cachedCfgPath.c_str());
            }
        }

        // ------------------------------ 辅助函数 ------------------------------

        /**
         * @brief 从文件加载JSON配置
         *
         * 读取指定路径的JSON文件并解析为rapidjson文档
         */
        bool ConfigManager::loadJsonFromFile(const std::string &path, rapidjson::Document &doc)
        {
            std::ifstream ifs(path);
            if (!ifs.is_open())
            {
                e_log("Open config file failed [%s]: %s", path.c_str(), strerror(errno));
                return false;
            }

            std::stringstream ss;
            ss << ifs.rdbuf();
            std::string content = ss.str();

            if (content.empty())
            {
                e_log("Config file is empty: %s", path.c_str());
                return false;
            }

            // Parse
            doc.Parse(content.c_str());

            // 3. 错误处理
            if (doc.HasParseError())
            {
                e_log("Parse config failed [%s]: %s, offset=%zu",
                          path.c_str(),
                          rapidjson::GetParseError_En(doc.GetParseError()),
                          doc.GetErrorOffset());
                return false;
            }

            // 必须为 object
            if (!doc.IsObject())
            {
                e_log("Config file [%s] root is not JSON object", path.c_str());
                return false;
            }

            return true;
        }

        /**
         * @brief 将JSON配置保存到文件
         *
         * 将rapidjson文档格式化保存到指定路径的文件中
         */
        bool ConfigManager::saveJsonToFile(const rapidjson::Document &doc, const std::string &path)
        {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);

            FILE *fp = fopen(path.c_str(), "w");
            if (!fp)
            {
                e_log("Open config file for writing failed [%s]: %s", path.c_str(), strerror(errno));
                return false;
            }

            size_t written = fwrite(buffer.GetString(), 1, buffer.GetSize(), fp);
            fclose(fp);

            if (written != buffer.GetSize())
            {
                e_log("Write config file failed [%s]: written=%zu expect=%zu", path.c_str(), written, buffer.GetSize());
                return false;
            }

            i_log("Saved config to: %s", path.c_str());
            return true;
        }

        /**
         * @brief 查找配置节点
         *
         * 根据层级路径（如"a.b.c"）查找对应的配置节点
         */
        bool ConfigManager::findNode(const std::string &key, const rapidjson::Value *&node) const
        {
            node = &m_currentDoc;
            size_t start = 0;
            size_t pos = key.find('.');
            std::string segment;

            while (true)
            {
                // 提取当前层级的键
                segment = (pos == std::string::npos) ? key.substr(start) : key.substr(start, pos - start);
                if (segment.empty())
                {
                    w_log("Invalid config key [%s]: empty segment", key.c_str());
                    return false;
                }

                // 检查当前节点是否为对象且包含该键
                if (!node->IsObject() || !node->HasMember(segment.c_str()))
                {
                    d_log("Config segment [%s] not found in key [%s]", segment.c_str(), key.c_str());
                    return false;
                }

                // 进入下一层节点
                node = &((*node)[segment.c_str()]);

                // 结束条件：已处理到最后一个层级
                if (pos == std::string::npos)
                {
                    break;
                }

                start = pos + 1;
                pos = key.find('.', start);
            }

            return true;
        }

        /**
         * @brief 查找或创建父节点
         *
         * 根据层级路径查找父节点，不存在的中间节点会自动创建
         */
        bool ConfigManager::findOrCreateParentNode(const std::string &key, rapidjson::Value *&parent, std::string &lastKey)
        {
            parent = &m_currentDoc;
            size_t start = 0;
            size_t pos = key.find('.');

            // 无层级键：父节点是根节点，lastKey是整个key
            if (pos == std::string::npos)
            {
                lastKey = key;
                return parent->IsObject();
            }

            // 处理层级键，自动创建中间节点
            while (true)
            {
                const std::string segment = key.substr(start, pos - start);
                if (segment.empty())
                {
                    e_log("Invalid config key [%s]: empty segment", key.c_str());
                    return false;
                }

                // 确保当前父节点是对象
                if (!parent->IsObject())
                {
                    e_log("Config segment [%s] is not object, cannot add child", segment.c_str());
                    return false;
                }

                // 不存在则创建空对象
                if (!parent->HasMember(segment.c_str()))
                {
                    d_log("Adding config segment [%s]", segment.c_str());
                    parent->AddMember(rapidjson::StringRef(segment.c_str()), rapidjson::Value(rapidjson::kObjectType), m_currentDoc.GetAllocator());
                }
                // 打印数据
                rapidjson::StringBuffer buffer;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                parent->Accept(writer);
                d_log("Current config: %s", buffer.GetString());
                // 进入下一层
                parent = &((*parent)[segment.c_str()]);

                // 检查是否为最后一个层级分隔符
                const size_t nextPos = key.find('.', pos + 1);
                if (nextPos == std::string::npos)
                {
                    lastKey = key.substr(pos + 1); // 最后一个层级的键
                    break;
                }

                start = pos + 1;
                pos = nextPos;
            }

            // 最终父节点必须是对象（才能添加子键）
            return parent->IsObject();
        }

        /**
         * @brief 获取所有祖先路径
         *
         * 根据修改路径生成所有可能的祖先路径（用于触发回调）
         * 例如："a.b.c" -> ["a", "a.b", "a.b.c"]
         */
        std::vector<std::string> getAncestorPaths(const std::string &modifyPath)
        {
            std::vector<std::string> paths;
            if (modifyPath.empty())
            {
                paths.emplace_back(""); // 全配置修改，仅自身
                return paths;
            }

            // 拆解路径：a.b.c → ["a", "a.b", "a.b.c"]
            size_t pos = 0;
            std::string currentPath;
            while (true)
            {
                pos = modifyPath.find('.', currentPath.empty() ? 0 : currentPath.size() + 1);
                std::string segment = (pos == std::string::npos) ? modifyPath.substr(currentPath.empty() ? 0 : currentPath.size() + 1)
                                                                 : modifyPath.substr(currentPath.empty() ? 0 : currentPath.size() + 1,
                                                                                     pos - (currentPath.empty() ? 0 : currentPath.size() + 1));

                currentPath = currentPath.empty() ? segment : currentPath + "." + segment;
                paths.emplace_back(currentPath);

                if (pos == std::string::npos)
                {
                    break;
                }
            }
            return paths;
        }

        /**
         * @brief 对比两个JSON对象的差异，返回被修改的子节点全路径
         * @param parentPath 父节点路径（如"ChargePoint"）
         * @param oldObj 旧的父节点对象
         * @param newObj 新的父节点对象
         * @return 被修改的子节点全路径列表（如["ChargePoint.ChargePointIdentifier"]）
         */
        std::vector<std::string> ConfigManager::diffJsonObjects(const std::string &parentPath,
                                                                const rapidjson::Value &oldObj,
                                                                const rapidjson::Value &newObj)
        {
            std::vector<std::string> changedPaths;
            if (!oldObj.IsObject() || !newObj.IsObject())
            {
                return changedPaths; // 非对象直接返回空（类型校验已保证是对象）
            }

            // 1. 遍历新对象的所有成员，对比旧对象
            for (auto it = newObj.MemberBegin(); it != newObj.MemberEnd(); ++it)
            {
                const std::string key = it->name.GetString();
                const std::string fullPath = parentPath.empty() ? key : (parentPath + "." + key);
                const rapidjson::Value &newValue = it->value;

                // 旧对象无此成员 → 新增节点（也算修改）
                if (!oldObj.HasMember(key.c_str()))
                {
                    changedPaths.push_back(fullPath);
                    continue;
                }

                const rapidjson::Value &oldValue = oldObj[key.c_str()];
                // 类型不同 → 修改
                if (oldValue.GetType() != newValue.GetType())
                {
                    changedPaths.push_back(fullPath);
                    continue;
                }

                // 递归对比子对象（如ChargePoint下还有嵌套对象的情况）
                if (newValue.IsObject())
                {
                    auto subChanged = diffJsonObjects(fullPath, oldValue, newValue);
                    changedPaths.insert(changedPaths.end(), subChanged.begin(), subChanged.end());
                    continue;
                }

                // 基础类型（字符串/数字/布尔）直接对比值
                bool isChanged = false;
                switch (newValue.GetType())
                {
                case rapidjson::kStringType:
                    isChanged = (strcmp(oldValue.GetString(), newValue.GetString()) != 0);
                    break;
                case rapidjson::kNumberType:
                    if (oldValue.IsInt() && newValue.IsInt())
                    {
                        isChanged = (oldValue.GetInt() != newValue.GetInt());
                    }
                    else if (oldValue.IsUint() && newValue.IsUint())
                    {
                        isChanged = (oldValue.GetUint() != newValue.GetUint());
                    }
                    else if (oldValue.IsDouble() && newValue.IsDouble())
                    {
                        isChanged = (oldValue.GetDouble() != newValue.GetDouble());
                    }
                    break;
                case rapidjson::kFalseType:
                case rapidjson::kTrueType:
                    isChanged = (oldValue.GetBool() != newValue.GetBool());
                    break;
                default:
                    isChanged = true; // 其他类型默认算修改
                    break;
                }

                if (isChanged)
                {
                    changedPaths.push_back(fullPath);
                }
            }

            // 2. 遍历旧对象的成员，新对象中不存在的（被删除的）→ 也算修改（可选，根据业务需求）
            // 若不需要处理删除场景，可注释此段
            for (auto it = oldObj.MemberBegin(); it != oldObj.MemberEnd(); ++it)
            {
                const std::string key = it->name.GetString();
                if (!newObj.HasMember(key.c_str()))
                {
                    const std::string fullPath = parentPath.empty() ? key : (parentPath + "." + key);
                    changedPaths.push_back(fullPath);
                }
            }

            return changedPaths;
        }

    } // namespace config
} // namespace ocpp1_6