#include "ConfigDefaults.h"
#include <string>
#include <vector>
#include "utils/utils.h"

namespace ocpp1_6
{
    namespace config
    {

        static const char *jsonTypeToString(rapidjson::Type t)
        {
            switch (t)
            {
            case rapidjson::kNullType:
                return "null";
            case rapidjson::kFalseType:
            case rapidjson::kTrueType:
                return "bool";
            case rapidjson::kObjectType:
                return "object";
            case rapidjson::kArrayType:
                return "array";
            case rapidjson::kStringType:
                return "string";
            case rapidjson::kNumberType:
                return "number";
            }
            return "unknown";
        }

        /**
         * @brief 递归校验配置节点
         *
         * 检查配置节点是否包含所有必需的键，并验证数据类型是否匹配
         *
         * @param defNode 默认配置节点
         * @param cfgNode 当前配置节点
         * @param path 当前路径（用于日志记录）
         * @return bool 配置是否有效
         */
        static bool checkNode(const rapidjson::Value &defNode, const rapidjson::Value &cfgNode, const std::string &path)
        {
            bool ok = true;

            // 遍历默认配置中的所有成员
            for (auto it = defNode.MemberBegin(); it != defNode.MemberEnd(); ++it)
            {
                const char *key = it->name.GetString();
                const rapidjson::Value &defVal = it->value;

                // 构建完整路径用于日志记录
                std::string fullPath = path.empty() ? key : path + "." + key;

                // 检查当前配置是否缺少该键
                if (!cfgNode.HasMember(key))
                {
                    w_log("Missing key: %s", fullPath.c_str());
                    ok = false;
                    continue;
                }

                // 获取当前配置中的对应值
                const rapidjson::Value &cfgVal = cfgNode[key];

                // 检查数据类型是否匹配
                bool same = (cfgVal.IsBool() && defVal.IsBool()) || (cfgVal.GetType() == defVal.GetType());
                if (!same)
                {
                    w_log("Type mismatch: %s (expect=%s, actual=%s)",
                          fullPath.c_str(),
                          jsonTypeToString(defVal.GetType()),
                          jsonTypeToString(cfgVal.GetType()));
                    ok = false;
                }

                // 如果是对象类型，递归检查子节点
                if (defVal.IsObject())
                {
                    if (!checkNode(defVal, cfgVal, fullPath))
                        ok = false;
                }
            }
            return ok;
        }

        /**
         * @brief 递归合并配置节点
         *
         * 将默认配置中的缺失项添加到当前配置中，并修复类型不匹配的项
         *
         * @param defNode 默认配置节点
         * @param cfgNode 当前配置节点（会被修改）
         * @param allocator 用于内存分配的分配器
         */
        static void mergeNode(const rapidjson::Value &defNode, rapidjson::Value &cfgNode, rapidjson::Document::AllocatorType &allocator)
        {
            // 遍历默认配置中的所有成员
            for (auto it = defNode.MemberBegin(); it != defNode.MemberEnd(); ++it)
            {
                const char *key = it->name.GetString();
                const rapidjson::Value &defVal = it->value;

                // 缺失 → 添加默认值
                if (!cfgNode.HasMember(key))
                {
                    rapidjson::Value k(key, allocator);
                    rapidjson::Value v(defVal, allocator);
                    cfgNode.AddMember(k, v, allocator);
                    i_log("Auto-add: %s", key);
                    continue;
                }

                // 获取当前配置中的对应值
                rapidjson::Value &cfgVal = cfgNode[key];

                // 类型不一致 → 覆盖为默认值
                bool same = (cfgVal.IsBool() && defVal.IsBool()) || (cfgVal.GetType() == defVal.GetType());
                if (!same)
                {
                    cfgVal.CopyFrom(defVal, allocator);
                    i_log("Auto-fix type for key: %s", key);
                    continue;
                }

                // 如果是对象类型，递归合并子节点
                if (defVal.IsObject())
                {
                    mergeNode(defVal, cfgVal, allocator);
                }
            }
        }

        /**
         * @brief 验证并合并配置
         *
         * 检查配置的有效性并用默认值填充缺失的项
         * 这是推荐使用的对外接口
         *
         * @param config 需要验证和合并的配置文档（会被修改）
         * @return bool 操作是否成功
         */
        bool validateAndMergeConfig(rapidjson::Document &config)
        {
            // 检查配置是否为有效的JSON对象
            if (!config.IsObject())
            {
                e_log("Invalid config JSON");
                return false;
            }

            // 解析默认配置
            rapidjson::Document def;
            def.Parse(DEFAULT_CONFIG_JSON);

            // 检查默认配置是否解析成功
            if (!def.IsObject())
            {
                e_log("Default config JSON parse error");
                return false;
            }

            // 先检查配置（记录日志）
            checkNode(def, config, "");

            // 再补全配置
            mergeNode(def, config, config.GetAllocator());

            return true;
        }

        bool CompareJsonTypeByPath(const std::string &keyPath, const rapidjson::Document &doc, const rapidjson::Value &target)
        {
            if (keyPath.empty())
            {
                return target.GetType() == rapidjson::kObjectType;
            }
            // 拆分 keyPath
            std::vector<std::string> keys;
            size_t start = 0, pos = 0;
            while ((pos = keyPath.find('.', start)) != std::string::npos)
            {
                keys.push_back(keyPath.substr(start, pos - start));
                start = pos + 1;
            }
            keys.push_back(keyPath.substr(start));

            // 从 doc 开始逐层查找
            const rapidjson::Value *current = &doc;
            for (const auto &k : keys)
            {
                if (!current->IsObject() || !current->HasMember(k.c_str()))
                {
                    return false;
                }
                current = &(*current)[k.c_str()];
            }

            bool same = (target.IsBool() && current->IsBool()) || (target.GetType() == current->GetType());
            if (!same)
            {
                w_log("Type mismatch: %s (expect=%s, actual=%s)",
                      keyPath.c_str(),
                      jsonTypeToString(target.GetType()),
                      jsonTypeToString(current->GetType()));
                return false;
            }
            return true;
        }

    } // namespace config
} // namespace ocpp1_6