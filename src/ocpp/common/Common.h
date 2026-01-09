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


#define RESERVATION "Reservation"
#define METERVALUES "MeterValues"
#define HEARTBEAT "Heartbeat"
#define BOOTNOTIFICATION "BootNotification"
#define STATUSNOTIFICATION "StatusNotification"


namespace ocpp1_6
{
    enum class MessageType
    {
        Call = 2,
        CallResult = 3,
        CallError = 4,
        Invalid = 5
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
}

#endif // COMMON_H
