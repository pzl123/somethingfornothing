#ifndef COMMON_H
#define COMMON_H

#include <random>
#include <chrono>

#include <string>
#include <stdexcept>
#include <climits>

#include <uuid/uuid.h>
#include "rapidjson/document.h"
#include "nlohmann/json.hpp"
namespace ocpp1_6
{
    enum class MessageType
    {
        Call = 2,
        CallResult = 3,
        CallError = 4,
        Invalid = 5
    };

    struct TypeCall
    {
        MessageType type;
        std::string uniqueId;
        std::string action;
        rapidjson::Document payload;
    };

    struct TypeCallResult
    {
        MessageType type;
        std::string uniqueId;
        std::string payload;
    };

    struct TypeCallError
    {
        MessageType type;
        std::string uniqueId;
        std::string errorCode;
        std::string errorDescription;
        rapidjson::Document errorDetails;
    };

    // URL信息结构
    struct UrlInfo
    {
        std::string protocol; // http, https, ftp, ftps
        std::string host;     // 主机名或IP
        int port = 0;         // 端口号
        std::string path;     // 路径
        std::string filename; // 文件名
        std::string username; // 用户名
        std::string password; // 密码
        std::string query;    // URL查询参数
    };

    /**
     * @brief 验证OCPP消息
     * @param jsonMessage - OCPP消息
     * @param type        - 消息类型
     * @param document    - 解析后的JSON文档
     * @return - 验证通过返回true，否则返回false
     */
    bool validateOCPPMessage(const std::string &jsonMessage, MessageType &type, rapidjson::Document &document);

    bool ocppisInt(const std::string &strVal);
    bool ocppisUint(const std::string &strVal);
    bool ocppisBool(const std::string &strVal);
    bool ocppstringToBool(const std::string &strVal);
    bool keyValueIsInt(const std::string &key);
    bool keyValueIsBool(const std::string& key);
    bool keyValueIsString(const std::string &key);

    /**
     * @brief jsonSerialize - 将JSON字符串转换为rapidjson::Document对象
     * @param jsonStr - JSON字符串
     * @param json   - rapidjson::Document对象
     * @return - 成功返回true，失败返回false
     */
    bool jsonSerialize(const std::string &jsonStr, rapidjson::Document &json);

    /**
     * @brief jsonDeserialize - 将rapidjson::Document对象转换为JSON字符串
     * @param odjJson - rapidjson::Document对象
     * @param jsonStr - JSON字符串
     * @return - 成功返回true，否则返回false
     */
    bool jsonDeserialize(const rapidjson::Document &json, std::string &str);

    /**
     * @brief generateMessageId - 生成OCPP消息唯一标识符
     * @return 返回生成的随机唯一消息ID字符串
     */
    std::string generateMessageId();

    /**
     * @brief generateOrderId - 生成订单ID(随机数字)
     * @return - 随机生成的订单ID
     */
    uint32_t generateOrderId();

    /**
     * @brief 递归创建目录
     * @param path - 目录路径
     * @return - 成功返回true，失败返回false
     */
    bool createDirRecursive(const std::string &path);

    // Energy.Active.Import.Register
    // 车从充电桩吸收的累计有功电能（充入电池的电量，单位kWh），能反映充了多少电。
    // 充电前后对比能知道充了多少电。
    typedef struct
    {
        std::string value;
        std::string unit;
    } EnergyActiveImportRegister;

    // Power.Active.Import
    // 当前瞬时充电功率（单位kW），能反映充电速度。
    // 方便监控充电是否正常，是否达到预期功率。
    typedef struct
    {
        std::string value;
        std::string unit;
    } PowerActiveImport;

    // Voltage
    // 当前充电电压（单位V），监控电压是否在正常范围内。
    typedef struct
    {
        std::string value;
        std::string unit;
    } Voltage;

    // Current.Import
    // 当前充电电流（单位A），实时反映充电电流大小。
    typedef struct
    {
        std::string value;
        std::string unit;
    } CurrentImport;

    // SoC
    // 电池当前的荷电状态（百分比），显示电池电量变化，能帮助判断充电进度。
    typedef struct
    {
        std::string value;
        std::string unit;
    } SoC;

    typedef struct
    {
        EnergyActiveImportRegister energyActiveImportRegister;
        PowerActiveImport powerActiveImport;
        Voltage voltage;
        CurrentImport currentImport;
        SoC soc;
    } SampledValue;

}

#endif // COMMON_H
