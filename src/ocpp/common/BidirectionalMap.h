// 添加头文件保护，防止重复定义
#ifndef BIDIRECTIONAL_MAP_H
#define BIDIRECTIONAL_MAP_H

#include <string>
#include <map>
#include <stdexcept>
#include <sstream>

namespace ocpp1_6
{

    // 双向映射类，用于枚举值与字符串的转换
    template <typename EnumType>
    class BidirectionalMap
    {
    public:
        // 构造函数，初始化时检查重复键
        BidirectionalMap(const std::initializer_list<std::pair<const std::string, EnumType>> &list)
        {
            for (const auto &pair : list)
            {
                // 检查重复的字符串键
                if (strToEnumMap.count(pair.first) > 0)
                {
                    std::stringstream ss;
                    ss << "BidirectionalMap: Duplicate string key '" << pair.first
                       << "' for enum value " << static_cast<int>(pair.second);
                    throw std::invalid_argument(ss.str());
                }
                // 检查重复的枚举值
                if (enumToStrMap.count(pair.second) > 0)
                {
                    std::stringstream ss;
                    ss << "BidirectionalMap: Duplicate enum value " << static_cast<int>(pair.second)
                       << " for string key '" << pair.first << "'";
                    throw std::invalid_argument(ss.str());
                }
                strToEnumMap[pair.first] = pair.second;
                enumToStrMap[pair.second] = pair.first;
            }
        }

        // 禁用复制构造函数和赋值运算符
        BidirectionalMap(const BidirectionalMap &) = delete;
        BidirectionalMap &operator=(const BidirectionalMap &) = delete;

        // 允许移动构造和移动赋值
        BidirectionalMap(BidirectionalMap &&) = default;
        BidirectionalMap &operator=(BidirectionalMap &&) = default;

        // ------------------------------
        // 旧版本接口（保持兼容，抛出异常）
        // ------------------------------

        // 字符串转枚举（旧版：不存在则抛出异常）
        EnumType toEnum(const std::string &str) const
        {
            auto it = strToEnumMap.find(str);
            if (it != strToEnumMap.end())
            {
                return it->second;
            }
            std::stringstream ss;
            ss << "BidirectionalMap: Invalid string '" << str
               << "' for enum conversion";
            throw std::invalid_argument(ss.str());
        }

        // 枚举转字符串（旧版：不存在则抛出异常）
        std::string toString(EnumType enumVal) const
        {
            auto it = enumToStrMap.find(enumVal);
            if (it != enumToStrMap.end())
            {
                return it->second;
            }
            std::stringstream ss;
            ss << "BidirectionalMap: Invalid enum value " << static_cast<int>(enumVal)
               << " for string conversion";
            throw std::invalid_argument(ss.str());
        }

        // ------------------------------
        // 新增安全版本接口（带默认值，不抛异常）
        // ------------------------------

        // 字符串转枚举（安全版：不存在则返回默认值）
        EnumType toEnum(const std::string &str, EnumType defaultValue) const noexcept
        {
            auto it = strToEnumMap.find(str);
            return (it != strToEnumMap.end()) ? it->second : defaultValue;
        }

        // 枚举转字符串（安全版：不存在则返回默认字符串）
        std::string toString(EnumType enumVal, const std::string &defaultValue) const noexcept
        {
            auto it = enumToStrMap.find(enumVal);
            return (it != enumToStrMap.end()) ? it->second : defaultValue;
        }

        // 检查字符串是否存在
        bool containsString(const std::string &str) const
        {
            return strToEnumMap.count(str) > 0;
        }

        // 检查枚举是否存在
        bool containsEnum(EnumType enumVal) const
        {
            return enumToStrMap.count(enumVal) > 0;
        }

    private:
        std::map<std::string, EnumType> strToEnumMap; // 字符串到枚举的映射
        std::map<EnumType, std::string> enumToStrMap; // 枚举到字符串的映射
    };

} // namespace ocpp1_6

// 头文件保护结束
#endif // BIDIRECTIONAL_MAP_H
